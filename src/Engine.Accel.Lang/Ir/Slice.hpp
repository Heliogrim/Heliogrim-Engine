#pragma once

#include <span>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Keeps what the required outputs (binding ids) need; an empty list keeps the module as it is. Never fails:
	 * an unknown id keeps nothing extra (Task 11 validates ids against the interface).
	 *
	 * - Outputs: those with a required id stay, all others are dropped, and so are the statements writing them.
	 *   Builtin targets (`position`, `frag_depth`) always count as kept.
	 * - Statements of a kept function: an assignment to a kept output, a builtin or anything but a local is live,
	 *   as is a `return`, the body root and every call statement or local definition whose callee may write such a
	 *   target. A local definition (`let x`, `x = ...`, `x.y = ...`) is live if a live statement reads `x` (by name,
	 *   ignoring scopes: sound, may keep too much); a block, `if` or `when` is live if a child is. Dead ones go.
	 * - Functions: `main` and, transitively, every function called from a live statement of a kept function. Without
	 *   a `main` with body, no function is kept.
	 * - Bindings: those a live statement touches, plus the required outputs (an implicit one needs no function).
	 * - Toggles and types are kept as they are; the arenas are rebuilt like `compact`, the input is not changed.
	 */
	[[nodiscard]] Module slice(ref<const Module> module_, std::span<const String> requiredOutputIds_);
}
