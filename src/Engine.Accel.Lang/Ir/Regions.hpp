#pragma once

#include <expected>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Lowers in-function region borrows to index arithmetic: `let r = &mut P[lo .. hi]` becomes
	 * `const r__off: u32 = lo; const r__len: u32 = hi - lo;`, `r[i]` becomes `P[r__off + i]` and `len(r)` becomes
	 * `r__len`. Existing nodes keep their index; new nodes are appended.
	 * A call passing regions to `F` is retargeted to a copy `F__r<k>` (one per callee name and static root of each region
	 * argument, numbered in first-call order) whose region param `p` becomes `p__off: u32, p__end: u32` (each bound passed once,
	 * `p__len` declared in the prologue); its body gets the
	 * same rewrite against the root, transitively. Region-taking originals are dropped (call `function` indices remapped).
	 * A root that is a local/parameter array or has a runtime index is a diagnostic.
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> lowerRegions(ref<const Module> module_);
}
