#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "BoxShapeCollisionComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::BoxShapeCollisionComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_MASK(::hg::BoxShapeCollisionComponent, settings, 0x03u),
			HG_LAYOUT_RAW(::hg::BoxShapeCollisionComponent, extent, ::hg::f32[3])
		};
	};
}
