#include "Level.hpp"

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "Engine.ACS/ActorModule.hpp"
#include "Engine.Serialization/SerializationModule.hpp"

import Engine.Common;
import Engine.Core;

using namespace ::hg::engine::serialization;
using namespace ::hg::engine::core;
using namespace ::hg;

template <>
void access::Structure<Level>::serialize(const Level& self_, mref<StructScopedSlot> record_) {
	Structure<math::Bounding>::serialize(self_._bounding, record_.insertStructSlot("bounding"));

	/**/

	const auto serial = Engine::getEngine()->getSerialization();
	auto sequence = record_.insertRecordSlot(Symbols::Components).intoSeq();

	for (const auto actor : self_.getActors()) {

		const auto actorTypeId = actor->getMetaClass()->typeId();
		auto fnSet = serial->getActorScheme(actorTypeId);
		::hg::assertrt(fnSet.has_value());

		fnSet->serialize(*actor, sequence.addRecordSlot().intoStruct());
	}
}

template <>
void access::Structure<Level>::hydrate(cref<StructScopedSlot> record_, Level& target_) {
	Structure<math::Bounding>::hydrate(record_.getStructSlot("bounding"), target_._bounding);

	/**/

	const auto sequence = record_.getRecordSlot(Symbols::Components).asSeq();
	const auto count = sequence.getRecordCount();
	if (count < 0LL) {
		// getRecordCount latched the malformed sequence on the load context.
		return;
	}

	/**/

	const auto serial = Engine::getEngine()->getSerialization();
	auto& registry = *Engine::getEngine()->getActors()->getRegistry();

	const auto& root = record_.slot()->getState().root;
	const auto ctx = root != nullptr ? root->context : nullptr;
	const auto failed = [&ctx] { return ctx != nullptr && ctx->failed(); };

	// Spec §3.4: actors are staged and only committed when the whole level loaded; dropping them destroys them.
	Vector<VolatileActor<>> staged {};
	Vector<ActorGuid> seen {};
	Vector<ComponentGuid> seenComponents {};
	// A contextless load drops `fail`, so the rejection is tracked here as well.
	bool rejected = false;

	for (s64 idx = 0; idx < count && not failed(); ++idx) {

		auto record = sequence.getRecordSlot(idx);
		const auto actor = record.asStruct();

		type_id actorTypeId {};
		actor.getSlot<u64>(Symbols::Type) >> actorTypeId.data;

		auto fnSet = serial->getActorScheme(actorTypeId);
		if (not fnSet.has_value()) {
			actor.warn(ReadError::Kind::eUnknownType, "actor type has no scheme; actor skipped");
			target_.markUnknownData();
			continue;
		}

		// R12: an identity appears once per load and never aliases a live actor.
		const auto guid = get_guid_slot<ActorGuid>(actor);
		if (std::ranges::contains(seen, guid) || registry.contains(guid)) {
			actor.slot()->fail(
				ReadError::Kind::eDuplicateIdentity,
				actor.slot()->getState().offset,
				0LL,
				"actor guid duplicates another actor of this load or a live actor"
			);
			rejected = true;
			break;
		}

		seen.emplace_back(guid);

		// R12 for component guids; archetype actors store their components as a struct, so only a seq is scanned.
		const auto components = actor.getRecordSlot(Symbols::Components).asSeq();
		if (components.slot()->getSlotHeader().type == StructureSlotType::eSeq) {
			const auto componentCount = components.getRecordCount();
			for (s64 cdx = 0; cdx < componentCount && not rejected; ++cdx) {
				const auto component = components.getRecordSlot(static_cast<u64>(cdx)).asStruct();
				if (not component.hasRecordSlot(Symbols::Guid)) {
					// The component scheme reports the missing guid.
					continue;
				}

				const auto componentGuid = get_guid_slot<ComponentGuid>(component);
				if (std::ranges::contains(seenComponents, componentGuid) || registry.contains(componentGuid)) {
					component.slot()->fail(
						ReadError::Kind::eDuplicateIdentity,
						component.slot()->getState().offset,
						0LL,
						"component guid duplicates another component of this load or a live component"
					);
					rejected = true;
				}

				seenComponents.emplace_back(componentGuid);
			}
			if (rejected) {
				break;
			}
		}

		auto re = fnSet->deserialize(record, std::addressof(registry));
		if (re == nullptr) {
			// Malformed or unknown actor record (reported on the load context); a save would drop it.
			target_.markUnknownData();
			continue;
		}

		staged.emplace_back(VolatileActor<> { ::hg::move(re) });
	}

	if (rejected || failed()) {
		// All-or-nothing: the load is rejected, so no staged actor may survive in the registry.
		staged.clear();
		return;
	}

	for (auto& actor : staged) {
		target_.addActor(::hg::move(actor));
	}

	if (record_.reportUnreadKeys()) {
		target_.markUnknownData();
	}
}
