#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "PointLightComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::PointLightComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::PointLightComponent, _distanceLimit, ::hg::f32),
			HG_LAYOUT_RAW(::hg::PointLightComponent, _luminance, ::hg::f32[3])
		};
	};
}
