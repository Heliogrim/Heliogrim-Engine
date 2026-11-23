#pragma once

#include <Engine.Reflow/Padding.hpp>
#include <Engine.Reflow/ReflowPlacement.hpp>
#include <Engine.Reflow/ReflowUnit.hpp>

#include "../LayoutAttribute.hpp"
#include "../LayoutAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct BoxLayout {
			constexpr static inline auto minHeight = "minHeight"sv;
			constexpr static inline auto maxHeight = "maxHeight"sv;

			constexpr static inline auto heightGrow = "heightGrow"sv;
			constexpr static inline auto heightShrink = "heightShrink"sv;

			constexpr static inline auto minWidth = "minWidth"sv;
			constexpr static inline auto maxWidth = "maxWidth"sv;

			constexpr static inline auto widthGrow = "widthGrow"sv;
			constexpr static inline auto widthShrink = "widthShrink"sv;

			constexpr static inline auto padding = "padding"sv;
			constexpr static inline auto placement = "placement"sv;
		};
	}

	struct BoxLayoutAttributes final :
		public LayoutAttributes<
			NamedAttribute<attr::BoxLayout::minWidth, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::maxWidth, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::widthGrow, LayoutAttribute<f32>>,
			NamedAttribute<attr::BoxLayout::widthShrink, LayoutAttribute<f32>>,
			/**/
			NamedAttribute<attr::BoxLayout::minHeight, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::maxHeight, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::heightGrow, LayoutAttribute<f32>>,
			NamedAttribute<attr::BoxLayout::heightShrink, LayoutAttribute<f32>>,
			NamedAttribute<attr::BoxLayout::padding, LayoutAttribute<Padding>>,
			/**/
			NamedAttribute<attr::BoxLayout::placement, LayoutAttribute<ReflowPlacement>>
		> {
		using LayoutAttributes::LayoutAttributes;
	};
}
