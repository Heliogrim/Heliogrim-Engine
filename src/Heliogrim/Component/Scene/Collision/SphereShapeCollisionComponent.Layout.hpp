#pragma once

#include <array>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "SphereShapeCollisionComponent.hpp"

namespace hg::engine::serialization::access {
	template <>
	class Layout<::hg::SphereShapeCollisionComponent> {
	public:
		inline constexpr static std::array fields {
			HG_LAYOUT_MASK(::hg::SphereShapeCollisionComponent, settings, 0x03u),
			HG_LAYOUT_RAW(::hg::SphereShapeCollisionComponent, radius, ::hg::f32)
		};
	};
}
