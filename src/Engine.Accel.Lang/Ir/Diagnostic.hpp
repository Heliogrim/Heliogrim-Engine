#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang::ir {
	enum class Severity : u8 {
		eNote,
		eWarning,
		eError
	};

	struct Diagnostic {
		Severity severity = Severity::eError;
		String message;
		u32 line = 0uL;
		u32 column = 0uL;
	};

	using Diagnostics = Vector<Diagnostic>;
}
