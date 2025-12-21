#include <Engine.ACS/ComponentGuid.hpp>
#include <Engine.ACS/ComponentTypeId.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "LogicComponent.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<LogicComponent>::serialize(
		const LogicComponent& self_,
		mref<StructScopedSlot> record_
	) {
		insert_typeId_slot<>(record_, self_._typeId);
		insert_guid_slot<>(record_, self_._guid);
	}

	template <>
	void access::Structure<LogicComponent>::hydrate(
		cref<StructScopedSlot> record_,
		ref<LogicComponent> target_
	) {
		target_._typeId = get_typeId_slot<ComponentTypeId>(record_);
		target_._guid = get_guid_slot<ComponentGuid>(record_);
	}
}
