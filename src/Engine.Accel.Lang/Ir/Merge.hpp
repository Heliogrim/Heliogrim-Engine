#pragma once

#include <expected>
#include <span>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Merges modules of one shader stage in injection order (profile modules first, the shader module last) into one
	 * module with the last module's name and entry kind. Deterministic for the same input order; holds no state.
	 *
	 * - Functions (by name): every declaration needs a body somewhere; bodies under structurally equal `@when`
	 *   conditions are the same variant, and a second body of a variant is an error unless the body it replaces is
	 *   `@redefine (allow = true)` (then the last wins). Forward declarations are dropped once satisfied; all
	 *   declarations of a name must agree on parameter and return types. Calls re-resolve by name.
	 * - Bindings (by id): same id collapses into one binding (condition OR-ed) if type, mode, class, direction
	 *   and `implicit` agree; any difference is an error naming both modules. Uses are re-pointed.
	 * - Toggles (by name): same value set and default collapse (first order kept); anything else is an error.
	 * - Types are deduplicated structurally (incl. bits, struct names and fields); input types may only refer to
	 *   earlier types. Any out-of-range ref in an input module is an error and nothing is merged.
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> merge(std::span<const Module* const> ordered_);
}
