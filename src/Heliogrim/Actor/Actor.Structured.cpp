#pragma once

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "Actor.hpp"
#include "Engine.Common/Move.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.Serialization/SerializationModule.hpp"
#include "Heliogrim/ActorInitializer.hpp"
#include "Heliogrim/Component/LogicComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Actor>::serialize(
	const Actor& self_,
	mref<StructScopedSlot> slot_
) {

	slot_.insertSlot<u64>(Symbols::Type) << Actor::typeId.data;

	/**/

	access::Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot(Symbols::Guid));

	/**/

	// TODO: Generic Serialize Components

	const auto serial = Engine::getEngine()->getSerialization();
	auto components = slot_.insertRecordSlot(Symbols::Components).intoSeq();

	u64 idx = 0;
	for (const auto& component : self_.getComponents()) {

		s8 recordParentIdx = -1;
		if (component->getParentComponent() != nullptr) {
			auto parentIt = self_.getComponents().find(component->getParentComponent());
			::hg::assertd(parentIt != self_.getComponents().end());
			recordParentIdx = static_cast<s8>(std::distance(self_.getComponents().begin(), parentIt));
		}

		/**/

		auto record = components.addRecordSlot().intoStruct();
		record.insertSlot<s8>(Symbols::Parent) << recordParentIdx;

		/**/

		const auto fnSet = serial->getComponentScheme(component->getTypeId());
		::hg::assertrt(fnSet.has_value());

		fnSet->serialize(*component, ::hg::move(record));
		++idx;
	}
}

template <>
void access::Structure<Actor>::hydrate(cref<StructScopedSlot> slot_, ref<Actor> target_) {

	type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == Actor::typeId);

	/**/

	access::Structure<Guid>::hydrate(slot_.getRecordSlot(Symbols::Guid).asStruct(), target_._guid);
}
