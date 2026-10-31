#pragma once

#include <expected>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	struct ToggleSources {
		ToggleSet pass;
		ToggleSet profile;
		ToggleSet material;
	};

	struct ResolveResult {
		/* every declared toggle, sorted by name */
		ToggleSet toggles;
		Diagnostics warnings;
	};

	/**
	 * Fixes one value per declared toggle: pass over profile over material over the module default. A pass value
	 * differing from the profile's, or a material value differing from the fixed one, is a warning (the fixed value
	 * wins). An unknown toggle, a value outside the declared domain or a toggle set twice in one source is an error
	 * naming the toggle and the source.
	 */
	[[nodiscard]] std::expected<ResolveResult, Diagnostics> fixToggles(ref<const Module> module_, ref<const ToggleSources> sources_);

	/**
	 * Resolves the module for one toggle combination (validated like a pass source; unset toggles take their
	 * default). Declarations whose condition is false are dropped, a `when` statement becomes its then (or else)
	 * arm, and all remaining conditions are `always`. Errors:
	 * - a function declared more than once with some condition must have a body for every combination of the
	 *   toggles its body conditions name (`no body for 'F' at A=x, B=y`, toggles in declaration order);
	 * - two bodies of one function both active (`overlapping bodies for 'F' at ...`, the full combination);
	 * - a call in surviving code to a function without surviving body (`no body for 'F' at ...`).
	 * Calls re-resolve by name and binding uses by binding (a dropped binding's uses go to the first surviving
	 * binding of the same local name). The arenas hold only the surviving code (see `compact`).
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> resolveToggles(ref<const Module> module_, ref<const ToggleSet> toggles_);
}
