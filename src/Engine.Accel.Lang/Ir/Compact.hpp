#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * What survives a rewrite of a module (shared by resolveToggles and slice); indices refer to the input module.
	 */
	struct CompactPlan {
		/* kept functions, ascending */
		Vector<u32> functions;
		/* kept bindings, ascending */
		Vector<u32> bindings;
		/* per input binding: the kept binding its uses go to (input index), ~0 = uses become unresolved */
		Vector<u32> bindingUses;
		/* per input statement: itself (keep), the statement replacing it (inline), or ~0 (drop) */
		Vector<u32> stmtAlias;
		/* kept constants, ascending; unset keeps all */
		Opt<Vector<u32>> constants {};
	};

	struct Compacted {
		Module mod;
		/* calls that resolve to no kept function, by signature (name when the ref is unset), unique, in arena order */
		Vector<String> unresolvedCalls;
	};

	/**
	 * Rebuilds the arenas from the kept function bodies only, in one canonical order (pre-order per body, functions
	 * in order, expressions after statements), so equal surviving code yields equal arenas whatever was dropped.
	 * Every ref of the result is in range or ~0: out-of-range refs become ~0, a back edge (cycle) is cut, a
	 * dropped statement leaves its block, and a dropped `if`/`when` arm or body root becomes an empty block.
	 * Calls keep their kept target, else re-resolve to the unique kept function with the same signature (name and parameter types).
	 */
	[[nodiscard]] Compacted compact(ref<const Module> mod_, ref<const CompactPlan> plan_);
}
