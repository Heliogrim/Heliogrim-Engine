#pragma once

#include <Engine.GFX/Color/Color.hpp>

#include "../StyleAttribute.hpp"
#include "../StyleAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct PaintStyleAttributes {
			constexpr static inline auto tint = "tint"sv;
		};
	}

	struct PaintStyleAttributes final :
		StyleAttributes<
			NamedAttribute<attr::PaintStyleAttributes::tint, StyleAttribute<color>>
		> {
		using StyleAttributes::StyleAttributes;
	};
}
