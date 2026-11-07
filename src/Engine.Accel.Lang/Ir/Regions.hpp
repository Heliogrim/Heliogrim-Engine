#pragma once

#include <expected>
#include <Engine.Common/Wrapper.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Lowers in-function region borrows to index arithmetic: `let r = &mut P[lo .. hi]` becomes
	 * `const r__off: u32 = lo; const r__len: u32 = hi - lo;`, `r[i]` becomes `P[r__off + i]` and `len(r)` becomes
	 * `r__len`. Existing nodes keep their index; new nodes are appended. Region parameters are left untouched.
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> lowerRegions(ref<const Module> module_);
}
