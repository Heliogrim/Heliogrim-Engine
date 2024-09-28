#include "Level.hpp"

#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "Engine.ACS/ActorModule.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.Serialization/SerializationModule.hpp"

using namespace ::hg::engine::serialization;
using namespace ::hg::engine::core;
using namespace ::hg;

template <>
void access::Structure<Level>::serialize(const Level& self_, mref<StructScopedSlot> slot_) {
	Structure<math::Bounding>::serialize(self_._bounding, slot_.insertStructSlot("bounding"));

	/**/

	const auto serial = Engine::getEngine()->getSerialization();
	auto sequence = slot_.insertRecordSlot(Symbols::Components).intoSeq();

	for (const auto actor : self_.getActors()) {

		const auto actorTypeId = actor->getMetaClass()->typeId();
		auto fnSet = serial->getActorScheme(actorTypeId);
		::hg::assertrt(fnSet.has_value());

		fnSet->serialize(*actor, sequence.addRecordSlot().intoStruct());
	}
}

template <>
void access::Structure<Level>::hydrate(cref<StructScopedSlot> slot_, Level& target_) {
	Structure<math::Bounding>::hydrate(slot_.getStructSlot("bounding"), target_._bounding);

	/**/

	const auto sequence = slot_.getRecordSlot(Symbols::Components).asSeq();
	::hg::assertrt(sequence.slot()->validateType());

	/**/

	const auto serial = Engine::getEngine()->getSerialization();
	auto& registry = *Engine::getEngine()->getActors()->getRegistry();

	const auto count = sequence.getRecordCount();
	for (s64 idx = 0; idx < count; ++idx) {

		auto record = sequence.getRecordSlot(idx);

		type_id actorTypeId {};
		record.asStruct().getSlot<u64>(Symbols::Type) >> actorTypeId.data;

		auto fnSet = serial->getActorScheme(actorTypeId);
		::hg::assertrt(fnSet.has_value());

		auto re = fnSet->deserialize(record, std::addressof(registry));
		::hg::assertrt(re != nullptr);

		target_.addActor(re);
	}
}
