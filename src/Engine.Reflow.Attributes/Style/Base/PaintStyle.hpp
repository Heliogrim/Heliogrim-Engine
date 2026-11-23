#pragma once

#include "../StyleAttribute.hpp"
#include "../StyleAttributes.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
#endif

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
