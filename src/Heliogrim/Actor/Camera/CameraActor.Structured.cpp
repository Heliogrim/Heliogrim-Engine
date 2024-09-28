#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "CameraActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<CameraActor>::serialize(
		const CameraActor& self_,
		mref<StructScopedSlot> slot_
	) {

		slot_.insertSlot<u64>(Symbols::Type) << CameraActor::typeId.data;

		/**/

		access::Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot(Symbols::Guid));
		access::Structure<CameraComponent>::serialize(*self_._cameraComponent, slot_.insertStructSlot("camera"));
		access::Structure<HierarchyComponent>::serialize(*self_._mountComponent, slot_.insertStructSlot("mount"));
	}

	template <>
	void access::Structure<CameraActor>::hydrate(cref<StructScopedSlot> slot_, ref<CameraActor> target_) {

		type_id checkTypeId {};
		slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
		::hg::assertrt(checkTypeId == CameraActor::typeId);

		/**/

		access::Structure<Guid>::hydrate(slot_.getStructSlot(Symbols::Guid), target_._guid);
		access::Structure<CameraComponent>::hydrate(slot_.getStructSlot("camera"), *target_._cameraComponent);
		access::Structure<HierarchyComponent>::hydrate(slot_.getStructSlot("mount"), *target_._mountComponent);
	}
}
