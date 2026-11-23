#include "LogicComponent.hpp"

#include <Engine.ACS/Component/ComponentGuid.hpp>
#include <Engine.ACS/Component/ComponentTypeId.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>
import Engine.Core;

import Engine.Serialization;

namespace hg::engine::serialization {
	template <>
	void access::Structure<LogicComponent>::serialize(
		const LogicComponent& self_,
		mref<StructScopedSlot> record_
	) {
		insert_typeId_slot<>(record_, self_._typeId);
		if (const auto engine = Engine::getEngine(); engine != nullptr) {
			engine->getSerialization()->migrations().writeRevision(record_, self_._typeId);
		}
		insert_guid_slot<>(record_, self_._guid);
		record_.insertSlot<String>("label"sv) << self_._label.text;
	}

	template <>
	void access::Structure<LogicComponent>::hydrate(
		cref<StructScopedSlot> record_,
		ref<LogicComponent> target_
	) {
		target_._typeId = get_typeId_slot<ComponentTypeId>(record_);
		target_._guid = get_guid_slot<ComponentGuid>(record_);
		record_.getSlot<String>("label"sv) >> target_._label.text;
	}
}
