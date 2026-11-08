#pragma once

#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	/**
	 * Direct usage of one function body; all lists sorted ascending and unique.
	 */
	struct Usage {
		Vector<u32> calledFunctions;
		Vector<u32> readBindings;
		Vector<u32> writtenBindings;
	};

	/**
	 * Both arms of a `when` count; names resolve by index, else by name (function name / BindingDecl::localName).
	 */
	[[nodiscard]] Usage usageOf(ref<const Module> mod_, u32 functionIndex_);

	/**
	 * Transitive closure over calls including the roots, ascending index order.
	 */
	[[nodiscard]] Vector<u32> reachableFunctions(ref<const Module> mod_, std::span<const u32> roots_);
}
