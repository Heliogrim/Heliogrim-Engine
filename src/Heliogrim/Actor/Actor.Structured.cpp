#pragma once

#include <algorithm>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>

#include "Actor.hpp"
#include "Archetype/ActorClass.hpp"
#include "../ActorInitializer.hpp"
#include "../Component/LogicComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Actor>::serialize(
	const Actor& self_,
	mref<StructScopedSlot> record_
) {

	insert_guid_slot(record_, self_._guid);
	insert_typeId_slot(record_, self_.getMetaClass<>()->typeId());
	record_.insertSlot<String>("label"sv) << self_._label.text;

	/**/

	if (const auto archetype = self_.getArchetype(); archetype != nullptr) {
		archetype->ops().serialize(
			reinterpret_cast<ptr<const std::byte>>(std::addressof(self_)) - archetype->desc().actorOffset,
			record_
		);
		return;
	}

	// Dynamic actors only: generic component sequence.

	const auto serial = Engine::getEngine()->getSerialization();
	auto components = record_.insertRecordSlot(Symbols::Components).intoSeq();

	u64 idx = 0;
	const auto actorComponents = self_.getComponents();
	for (auto* const component : actorComponents) {

		s8 recordParentIdx = -1;
		if (component->getParentComponent() != nullptr) {
			const auto parentIt = std::ranges::find(actorComponents, component->getParentComponent());
			::hg::assertd(parentIt != actorComponents.end());
			recordParentIdx = static_cast<s8>(std::distance(actorComponents.begin(), parentIt));
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
void access::Structure<Actor>::hydrate(cref<StructScopedSlot> record_, ref<Actor> target_) {

	const auto checkTypeId = get_typeId_slot(record_);
	::hg::assertrt(checkTypeId == target_.getMetaClass<>()->typeId());

	/**/

	target_._guid = get_guid_slot<ActorGuid>(record_);
	record_.getSlot<String>("label"sv) >> target_._label.text;

	/**/

	if (const auto archetype = target_.getArchetype(); archetype != nullptr) {
		archetype->ops().hydrate(
			reinterpret_cast<ptr<std::byte>>(std::addressof(target_)) - archetype->desc().actorOffset,
			record_
		);
	}
}
