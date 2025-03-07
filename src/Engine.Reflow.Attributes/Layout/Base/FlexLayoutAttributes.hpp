#pragma once

#include <Engine.Reflow/ReflowAlignment.hpp>
#include <Engine.Reflow/ReflowSpacing.hpp>

#include "../LayoutAttribute.hpp"
#include "../LayoutAttributes.hpp"

namespace hg::engine::reflow {
	namespace attr {
		struct FlexLayout {
			constexpr static inline auto colGap = "columnGap"sv;
			constexpr static inline auto rowGap = "rowGap"sv;

			constexpr static inline auto align = "align"sv;
			constexpr static inline auto justify = "justify"sv;
		};
	}

	struct FlexLayoutAttributes final :
		public LayoutAttributes<
			NamedAttribute<attr::FlexLayout::colGap, LayoutAttribute<float>>,
			NamedAttribute<attr::FlexLayout::rowGap, LayoutAttribute<float>>,
			NamedAttribute<attr::FlexLayout::align, LayoutAttribute<ReflowAlignment>>,
			NamedAttribute<attr::FlexLayout::justify, LayoutAttribute<ReflowSpacing>>
		> {
		using LayoutAttributes::LayoutAttributes;
	};
}
