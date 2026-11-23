#include "CameraComponent.hpp"

#include <Engine.Serialization.Structures/Symbols.hpp>

import Engine.Common;
import Engine.Serialization;

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<CameraComponent>::serialize(cref<CameraComponent> self_, mref<StructScopedSlot> record_) {

	record_.insertSlot<f32>("fov") << self_._fov;
	record_.insertSlot<f32>("aspectRation") << self_._aspectRation;
	record_.insertSlot<bool>("lockedAspect") << self_._lockedAspect;
	record_.insertSlot<bool>("visibleFrustum") << self_._visibleFrustum;
	Structure<math::fvec3>::serialize(*std::addressof(self_._offset), record_.insertStructSlot("offset"));
	record_.insertSlot<std::underlying_type_t<graphics::CameraProjectionMode>>("projectMode") << self_._projectMode;
	record_.insertSlot<std::underlying_type_t<graphics::CameraViewMode>>("viewMode") << self_._viewMode;

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<CameraComponent>::hydrate(cref<StructScopedSlot> record_, CameraComponent& target_) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	record_.getSlot<f32>("fov") >> target_._fov;
	record_.getSlot<f32>("aspectRation") >> target_._aspectRation;
	record_.getSlot<bool>("lockedAspect") >> target_._lockedAspect;
	record_.getSlot<bool>("visibleFrustum") >> target_._visibleFrustum;
	Structure<math::fvec3>::hydrate(record_.getStructSlot("offset"), target_._offset);
	record_.getSlot<std::underlying_type_t<graphics::CameraProjectionMode>>("projectMode") >> target_._projectMode;
	record_.getSlot<std::underlying_type_t<graphics::CameraViewMode>>("viewMode") >> target_._viewMode;
}
