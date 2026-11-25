#pragma once

#include <Engine.Reflow/Padding.hpp>
#include <Engine.Reflow/ReflowUnit.hpp>

#include "../LayoutAttribute.hpp"
#include "../LayoutAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct GridCell {
			inline constexpr static auto cellMinWidth = "cellMinWidth"sv;
			inline constexpr static auto cellMaxWidth = "cellMaxWidth"sv;

			inline constexpr static auto cellMinHeight = "cellMinHeight"sv;
			inline constexpr static auto cellMaxHeight = "cellMaxHeight"sv;

			inline constexpr static auto cellPadding = "cellPadding"sv;
		};
	}

	struct GridCellAttributes final :
		public LayoutAttributes<
			NamedAttribute<attr::GridCell::cellMinWidth, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::GridCell::cellMaxWidth, LayoutAttribute<ReflowUnit>>,
			/**/
			NamedAttribute<attr::GridCell::cellMinHeight, LayoutAttribute<ReflowUnit>>,
			NamedAttribute<attr::GridCell::cellMaxHeight, LayoutAttribute<ReflowUnit>>,
			/**/
			NamedAttribute<attr::GridCell::cellPadding, LayoutAttribute<Padding>>
		> {
		using LayoutAttributes::LayoutAttributes;
	};
}
