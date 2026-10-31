#pragma once

#include <span>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Keeps what the required outputs (binding ids) need; an empty list keeps the module as it is. Never fails:
	 * an unknown id keeps nothing extra (Task 11 validates ids against the interface). Expects a module resolved by
	 * `resolveToggles` (resolve order: toggles, then slice): on unresolved input the parse-time binding and call refs
	 * of `@when` variants may name the wrong variant.
	 *
	 * - Outputs: those with a required id stay, all others are dropped, and so are the statements writing them
	 *   unless their expressions call a writer (below); a dropped output that live code writes or reads is kept.
	 *   Builtin targets (`position`, `frag_depth`) always count as kept.
	 * - Statements of a kept function: an assignment to a kept output, a builtin or anything but a local is live,
	 *   as is a `return` and the body root. Any statement (assignment, `if`/`when` condition, call statement, local
	 *   definition) whose expressions call a function that may write such a target is live. A local definition (`let x`, `x = ...`, `x.y = ...`) is live if a live statement reads `x` (by name,
	 *   ignoring scopes: sound, may keep too much); a block, `if` or `when` is live if a child is. Dead ones go.
	 * - Functions: `main` and, transitively, every function called from a live statement of a kept function. Without
	 *   a `main` with body, no function is kept.
	 * - Bindings: those a live statement touches, plus the required outputs (an implicit one needs no function).
	 * - Toggles and types are kept as they are; the arenas are rebuilt like `compact`, the input is not changed.
	 */
	[[nodiscard]] Module slice(ref<const Module> module_, std::span<const String> requiredOutputIds_);
}
