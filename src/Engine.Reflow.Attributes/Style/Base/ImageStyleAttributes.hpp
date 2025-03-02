#pragma once

#include <Engine.GFX/Color/Color.hpp>
#include <Engine.Reflow/ReflowAlpha.hpp>

#include "../StyleAttribute.hpp"
#include "../StyleAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct ImageStyle {
			constexpr static inline auto alpha = "alpha"sv;
			constexpr static inline auto tint = "tint"sv;
		};
	}

	struct ImageStyleAttributes final :
		StyleAttributes<
			NamedAttribute<attr::ImageStyle::alpha, StyleAttribute<ReflowAlphaMode>>,
			NamedAttribute<attr::ImageStyle::tint, StyleAttribute<color>>
		> {
		using StyleAttributes::StyleAttributes;
	};
}
