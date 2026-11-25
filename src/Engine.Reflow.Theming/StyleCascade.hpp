#pragma once

#include "ComponentStyle.hpp"
#include "ReflowClassList.hpp"
#include "StyleRule.hpp"
#include "ThemeContext.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	/**
	 * The component style of the first static class name that has one.
	 */
	[[nodiscard]] Opt<ref<const ComponentStyle>> findComponentStyle(
		ref<const ThemeContext> context_,
		ref<const ReflowClassList> classList_
	);

	/**
	 * The legacy per-set theme entry of the first static class name that has one.
	 */
	template <class Set_>
	[[nodiscard]] Opt<ref<const Set_>> findLegacyStyle(ref<const ThemeContext> context_, ref<const ReflowClassList> classList_) {
		return FirstOf<Opt<ref<const Set_>>>(
			classList_,
			[&](const auto& className_) -> Opt<ref<const Set_>> {
				if (not className_.template is<StringView>()) {
					return None;
				}

				const auto name = className_.template as<StringView>();
				if constexpr (std::is_same_v<Set_, BoxLayoutAttributes>) {
					return context_.getBoxLayout(name);

				} else if constexpr (std::is_same_v<Set_, PaintStyleAttributes>) {
					return context_.getPaintStyle(name);

				} else if constexpr (std::is_same_v<Set_, TextStyleAttributes>) {
					return context_.getTextStyle(name);

				} else {
					static_assert(std::is_same_v<Set_, ImageStyleAttributes>, "No legacy theme entry for this attribute set.");
					return context_.getImageStyle(name);
				}
			}
		);
	}

	/**
	 * Resolves `inOut_` through: stored value, the `LegacySlots_` of the legacy theme entry, component base rule, matching state layers in
	 * order, then `instance_`. The result is written once, so unchanged slots mark nothing.
	 *
	 * shortcut: no per-key cache, resolution runs on every cascadeContextChange; cache per (component, states) if it shows in profiles.
	 */
	template <class Set_, ref<const StringView>... LegacySlots_>
	void resolveStyle(
		ref<const ThemeContext> context_,
		ref<const ReflowClassList> classList_,
		const StyleStates states_,
		ref<const StyleRule<Set_>> instance_,
		ref<Set_> inOut_
	) {
		StyleRule<Set_> resolved {};

		if (const auto legacy = findLegacyStyle<Set_>(context_, classList_); legacy != None) {
			StyleRule<Set_>::template pick<LegacySlots_...>(legacy.value()).applyTo(resolved);
		}

		if (const auto component = findComponentStyle(context_, classList_); component != None) {
			ruleOf<Set_>(component.value()).applyTo(resolved);
			for (const auto& layer : component->states) {
				if (stateLayerMatches(layer.mask, states_)) {
					ruleOf<Set_>(layer).applyTo(resolved);
				}
			}
		}

		instance_.applyTo(resolved);
		resolved.applyTo(inOut_);
	}
}
