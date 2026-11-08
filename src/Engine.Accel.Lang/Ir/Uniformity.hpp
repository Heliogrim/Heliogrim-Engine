#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Module.hpp"

namespace hg::engine::accel::lang::ir {
	struct Uniformity {
		/* by StmtRef: false when the statement may run under lane-dependent control */
		Vector<bool> uniformStmt;
		/* by FunctionRef: contains (directly or through calls) a builtin that needs uniform control flow */
		Vector<bool> requiresUniform;
	};

	/**
	 * @param needsUniform_ true for builtin names (variable or call) that are only valid under uniform control; default none
	 */
	[[nodiscard]] Uniformity analyzeUniformity(
		ref<const Module> mod_,
		FnRef<bool(StringView)> needsUniform_ = [](StringView) { return false; }
	);
}
