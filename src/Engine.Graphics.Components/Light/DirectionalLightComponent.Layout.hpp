#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "DirectionalLightComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::DirectionalLightComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_RAW(::hg::DirectionalLightComponent, _luminance, ::hg::f32[3]),
			HG_LAYOUT_RAW(::hg::DirectionalLightComponent, _direction, ::hg::f32[3])
		};
	};
}
