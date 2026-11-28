#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Diagnostics;
#endif

namespace hg::editor::codes {
	// numbers are frozen once published
	inline constexpr engine::diag::DiagnosticCodeInfo table[] = {
		{ { { { "EDT" } }, 1 }, "no-root-level", "the current universe has no root level" },
		{ { { { "EDT" } }, 2 }, "level-asset-unregistered", "no level asset is registered for the root level" }
	};

	consteval ref<const engine::diag::DiagnosticCodeInfo> edt(const StringView key_) {
		return engine::diag::findCode(table, key_);
	}
}
