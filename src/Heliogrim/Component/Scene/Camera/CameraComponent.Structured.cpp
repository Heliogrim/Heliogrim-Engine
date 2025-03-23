#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "CameraComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<CameraComponent>::serialize(cref<CameraComponent> self_, mref<StructScopedSlot> slot_) {

	slot_.insertSlot<u64>(Symbols::Type) << CameraComponent::typeId.data;

	/**/

	Structure<math::Bounding>::serialize(self_._boundaries, slot_.insertStructSlot("boundaries"));
	Structure<math::Transform>::serialize(self_._localTransform, slot_.insertStructSlot("localTransform"));

	/**/

	slot_.insertSlot<f32>("fov") << self_._fov;
	slot_.insertSlot<f32>("aspectRation") << self_._aspectRation;
	slot_.insertSlot<bool>("lockedAspect") << self_._lockedAspect;
	slot_.insertSlot<bool>("visibleFrustum") << self_._visibleFrustum;
	Structure<math::fvec3>::serialize(*std::addressof(self_._offset), slot_.insertStructSlot("offset"));
	slot_.insertSlot<std::underlying_type_t<gfx::CameraProjectionMode>>("projectMode") << self_._projectMode;
	slot_.insertSlot<std::underlying_type_t<gfx::CameraViewMode>>("viewMode") << self_._viewMode;

}

template <>
void access::Structure<CameraComponent>::hydrate(cref<StructScopedSlot> slot_, CameraComponent& target_) {

	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == CameraComponent::typeId);

	/**/

	Structure<math::Bounding>::hydrate(slot_.getStructSlot("boundaries"), target_._boundaries);
	Structure<math::Transform>::hydrate(slot_.getStructSlot("localTransform"), target_._localTransform);

	/**/

	slot_.getSlot<f32>("fov") >> target_._fov;
	slot_.getSlot<f32>("aspectRation") >> target_._aspectRation;
	slot_.getSlot<bool>("lockedAspect") >> target_._lockedAspect;
	slot_.getSlot<bool>("visibleFrustum") >> target_._visibleFrustum;
	Structure<math::fvec3>::hydrate(slot_.getStructSlot("offset"), target_._offset);
	slot_.getSlot<std::underlying_type_t<gfx::CameraProjectionMode>>("projectMode") >> target_._projectMode;
	slot_.getSlot<std::underlying_type_t<gfx::CameraViewMode>>("viewMode") >> target_._viewMode;
}
