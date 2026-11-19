#pragma once

#include <Engine.Common/__macro.hpp>

#include "Diagnostic.hpp"
#include "Module.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir {
	/**
	 * Lowers references to root + access path; the result holds no reachable `eRange`, region-typed value or `eDeclareRef`.
	 * - `let a = &mut P` disappears: each runtime index of `P` is frozen into `const a__i<n>` at the borrow and every use
	 *   of `a` becomes the full place. A region borrow `&mut P[lo .. hi]` becomes `const a__off`/`a__len`, `a[i]` becomes
	 *   `P[a__off + i]` and `len(a)` becomes `a__len`.
	 * - A call passing a reference or region to a function with a body is retargeted to a specialisation (empty `name`,
	 *   `origin` = the source function's name, `loc` its location), one per callee and per argument shape:
	 *   - global root (binding, workgroup, constant, builtin): the param is removed; each runtime index of the path becomes
	 *     a `u32` param `p__i<k>` and every use of `p` the full place from the root (key: root + constant path prefix);
	 *   - local root (local or parameter): the param keeps its kind but takes the WHOLE root's type, followed by `u32`
	 *     params `p__i<k>`; uses of `p` become the path from that param (key: root type + path shape);
	 *   - region: `p` becomes `p__off: u32, p__end: u32` (`p__len` in the prologue) against the root, which must be a
	 *     global without runtime indices;
	 *   Runtime indices convert to `u32` at the call site, so the key ignores their type; literal indices key by value.
	 *   A non-place argument to a `&` param (a literal) passes by value; one to `&mut` is a typecheck bug (asserted).
	 *   Bodies get the same rewrite, transitively. Originals taking references or regions are dropped (call `function`
	 *   indices remapped); kept functions have an empty `origin`.
	 * Existing nodes keep their index; new nodes are appended.
	 */
	[[nodiscard]] std::expected<Module, Diagnostics> lowerReferences(ref<const Module> mod_);
}
