#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "CameraComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::CameraComponent> {
	public:
		// The modes are one-byte enums of 0 or 1, streamed as a masked byte.
		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::CameraComponent, _fov, ::hg::f32),
			HG_LAYOUT_RAW(::hg::CameraComponent, _aspectRation, ::hg::f32),
			HG_LAYOUT_MASK(::hg::CameraComponent, _lockedAspect, 0x01u),
			HG_LAYOUT_MASK(::hg::CameraComponent, _visibleFrustum, 0x01u),
			HG_LAYOUT_RAW(::hg::CameraComponent, _offset, ::hg::f32[3]),
			HG_LAYOUT_MASK(::hg::CameraComponent, _projectMode, 0x01u),
			HG_LAYOUT_MASK(::hg::CameraComponent, _viewMode, 0x01u)
		};
	};
}
