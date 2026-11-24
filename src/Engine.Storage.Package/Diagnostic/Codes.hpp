
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Diagnostics;
#endif
#pragma once


namespace hg::engine::storage::package::codes {
	// numbers are frozen once published
	inline constexpr diag::DiagnosticCodeInfo table[] = {
		{ { { { "STO" } }, 1 }, "package-malformed", "package header, sizes, index or guids fail the open checks" },
		{ { { { "STO" } }, 2 }, "package-checksum", "package CRC32 differs from the stored one" },
		{ { { { "STO" } }, 3 }, "package-newer", "package header version is above the reader's" }
	};

	consteval ref<const diag::DiagnosticCodeInfo> sto(const StringView key_) {
		return diag::findCode(table, key_);
	}
}
