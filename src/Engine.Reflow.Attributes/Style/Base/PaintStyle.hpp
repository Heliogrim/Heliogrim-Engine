#pragma once

#include <Engine.GFX/Color/Color.hpp>

#include "../StyleAttribute.hpp"
#include "../StyleAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct PaintStyle {
			constexpr static inline auto tint = "tint"sv;
			constexpr static inline auto borderRadius = "borderRadius"sv;
		};
	}

	struct PaintStyleAttributes final :
		StyleAttributes<
			NamedAttribute<attr::PaintStyle::tint, StyleAttribute<color>>,
			NamedAttribute<attr::PaintStyle::borderRadius, StyleAttribute<math::fvec4>>
		> {
		using StyleAttributes::StyleAttributes;
	};
}
