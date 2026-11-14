#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Diagnostics/Diagnostic.hpp>

#include "../Diagnostic/Codes.hpp"

namespace hg::engine::accel::lang::ir {
	using Severity = diag::Severity;
	using Diagnostic = diag::Diagnostic;
	using Diagnostics = diag::Diagnostics;
	using DiagnosticCodeInfo = diag::DiagnosticCodeInfo;
	using NoLocation = diag::NoLocation;
	using SourceSpan = diag::SourceSpan;

	/**
	 * 1-based line and column; a tab counts as one column.
	 */
	struct SourceLoc {
		u32 line = 1uL;
		u32 column = 1uL;
	};

	/* a point span at `loc_`, for sites that know no byte range */
	[[nodiscard]] diag::SourceSpan pointAt(StringView file_, SourceLoc loc_);
}
