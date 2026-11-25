#pragma once

#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/ImageStyleAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/PaintStyle.hpp>
#include <Engine.Reflow.Attributes/Style/Base/TextStyleAttributes.hpp>

#include "StyleRule.hpp"
#include "StyleStates.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	/**
	 * Overrides that apply while every bit of `mask` is set in the widget's style states.
	 */
	struct StateLayer {
		StyleStates mask;

		StyleRule<BoxLayoutAttributes> box;
		StyleRule<PaintStyleAttributes> paint;
		StyleRule<TextStyleAttributes> text;
		StyleRule<ImageStyleAttributes> image;
	};

	struct ComponentStyle {
		StyleRule<BoxLayoutAttributes> box;
		StyleRule<PaintStyleAttributes> paint;
		StyleRule<TextStyleAttributes> text;
		StyleRule<ImageStyleAttributes> image;

		/**
		 * Kept sorted by `stateLayerOrder`; later layers win.
		 */
		Vector<StateLayer> states;
	};

	/**
	 * Sort key of a state layer: the highest-precedence bit of the mask first, then the bit count.
	 * Precedence from low to high: selected, hover, focus, press, drag, invalid, disable.
	 */
	[[nodiscard]] u32 stateLayerOrder(const StyleStates mask_) noexcept;

	[[nodiscard]] bool stateLayerMatches(const StyleStates mask_, const StyleStates states_) noexcept;

	template <class Set_, class Holder_>
	[[nodiscard]] ref<const StyleRule<Set_>> ruleOf(ref<const Holder_> holder_) noexcept {
		if constexpr (std::is_same_v<Set_, BoxLayoutAttributes>) {
			return holder_.box;

		} else if constexpr (std::is_same_v<Set_, PaintStyleAttributes>) {
			return holder_.paint;

		} else if constexpr (std::is_same_v<Set_, TextStyleAttributes>) {
			return holder_.text;

		} else {
			static_assert(std::is_same_v<Set_, ImageStyleAttributes>, "No component style rule for this attribute set.");
			return holder_.image;
		}
	}
}
