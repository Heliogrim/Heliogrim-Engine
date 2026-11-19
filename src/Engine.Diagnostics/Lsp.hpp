#pragma once

#include <Engine.Common/__macro.hpp>

#include "Diagnostic.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::diag {
	inline constexpr StringView codeDocsBase = "https://heliogrim.github.io/Heliogrim-Pages/reference/diagnostics/";

	/**
	 * LSP 3.17 `Diagnostic` object with a fixed key order.
	 */
	[[nodiscard]] String toLspJson(ref<const Diagnostic> diagnostic_);

	/**
	 * JSON array of LSP `CodeAction` objects, `[]` without fixes.
	 */
	[[nodiscard]] String toLspCodeActionsJson(ref<const Diagnostic> diagnostic_);

	/**
	 * Document uri for PublishDiagnosticsParams (the LSP `Diagnostic` has no uri field); empty without a uri-bearing primary.
	 */
	[[nodiscard]] String lspUriOf(ref<const Diagnostic> diagnostic_);
}
