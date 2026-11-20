#pragma once

#include <Engine.Diagnostics/Diagnostic.hpp>

#include "../Diagnostic/Codes.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir {
	using Severity = diag::Severity;
	using Diagnostic = diag::Diagnostic;
	using Diagnostics = diag::Diagnostics;
	using DiagnosticCodeInfo = diag::DiagnosticCodeInfo;
	using NoLocation = diag::NoLocation;
	using SourceSpan = diag::SourceSpan;

	/**
	 * 1-based line and column; a tab counts as one column. `offset` and `length` are the byte range in the module source `source`.
	 */
	struct SourceLoc {
		u32 line = 1uL;
		u32 column = 1uL;
		u32 offset = 0uL;
		u32 length = 0uL;
		/* index into `Module::sources`: the source whose bytes the range indexes */
		u16 source = 0u;
	};

	/* the span of `loc_`'s byte range; a point when the range is empty */
	[[nodiscard]] diag::SourceSpan pointAt(StringView file_, SourceLoc loc_);
}
