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
			constexpr static inline auto height = "height"sv;
			constexpr static inline auto maxHeight = "maxHeight"sv;

			constexpr static inline auto minWidth = "minWidth"sv;
			constexpr static inline auto width = "width"sv;
			constexpr static inline auto maxWidth = "maxWidth"sv;

			constexpr static inline auto padding = "padding"sv;
			constexpr static inline auto placement = "placement"sv;
		};
	}

	struct BoxLayoutAttributes final :
		public LayoutAttributes<
			NamedAttribute<attr::BoxLayout::minWidth, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::width, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::maxWidth, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::minHeight, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::height, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::maxHeight, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::BoxLayout::padding, LayoutAttribute<Padding>>,
			NamedAttribute<attr::BoxLayout::placement, LayoutAttribute<ReflowPlacement>>
		> {
		using LayoutAttributes::LayoutAttributes;
	};
}
