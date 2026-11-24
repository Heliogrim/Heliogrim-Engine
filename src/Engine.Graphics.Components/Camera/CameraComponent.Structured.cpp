#include "CameraComponent.hpp"

import Engine.Common;
import Engine.Serialization;
import Engine.Serialization.Structures;

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<CameraComponent>::serialize(cref<CameraComponent> self_, mref<StructScopedSlot> record_) {

	record_.insertSlot<f32>("fov") << self_._fov;
	record_.insertSlot<f32>("aspectRation") << self_._aspectRation;
	record_.insertSlot<bool>("lockedAspect") << self_._lockedAspect;
	record_.insertSlot<bool>("visibleFrustum") << self_._visibleFrustum;
	Structure<math::fvec3>::serialize(*std::addressof(self_._offset), record_.insertStructSlot("offset"));
	// The slots stay s32 so records saved before the enums became one byte still load.
	record_.insertSlot<s32>("projectMode") << static_cast<s32>(self_._projectMode);
	record_.insertSlot<s32>("viewMode") << static_cast<s32>(self_._viewMode);

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
	{
		auto value = s32 {};
		record_.getSlot<s32>("projectMode") >> value;
		if (value != 0 && value != 1) {
			record_.slot()->fail(
				ReadError::Kind::eInvalidValue,
				record_.slot()->getState().offset,
				0LL,
				"camera projection mode is out of range"
			);
			return;
		}
		target_._projectMode = static_cast<graphics::CameraProjectionMode>(value);
	}
	{
		auto value = s32 {};
		record_.getSlot<s32>("viewMode") >> value;
		if (value != 0 && value != 1) {
			record_.slot()->fail(
				ReadError::Kind::eInvalidValue,
				record_.slot()->getState().offset,
				0LL,
				"camera view mode is out of range"
			);
			return;
		}
		target_._viewMode = static_cast<graphics::CameraViewMode>(value);
	}
}
