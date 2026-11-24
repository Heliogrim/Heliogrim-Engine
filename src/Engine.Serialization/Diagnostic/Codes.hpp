
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Diagnostics;
#endif
#pragma once


namespace hg::engine::serialization::codes {
	// numbers are frozen once published
	inline constexpr diag::DiagnosticCodeInfo table[] = {
		{ { { { "SER" } }, 1 }, "malformed-slot", "size, tag, key or count violates the slot rules" },
		{ { { { "SER" } }, 2 }, "truncated", "data ends inside a slot, or the archive reported a read error" },
		{ { { { "SER" } }, 3 }, "nesting-too-deep", "nesting depth is above 64" },
		{ { { { "SER" } }, 4 }, "limit-exceeded", "the load's allocation budget is used up" },
		{ { { { "SER" } }, 5 }, "type-mismatch", "stored tag cannot convert to the requested type" },
		{ { { { "SER" } }, 6 }, "invalid-value", "bool not 0 or 1, enum or index out of range" },
		{ { { { "SER" } }, 7 }, "duplicate-key", "a key appears twice in one struct" },
		{ { { { "SER" } }, 8 }, "missing-field", "a required key is absent" },
		{ { { { "SER" } }, 9 }, "duplicate-identity", "a guid appears twice or is already live" },
		{ { { { "SER" } }, 10 }, "unknown-type", "type is not registered, the record is skipped" },
		{ { { { "SER" } }, 11 }, "unknown-field", "a key the type did not read" },
		{ { { { "SER" } }, 12 }, "revision-newer", "stored revision is above the type's, the record is skipped" },
		{ { { { "SER" } }, 13 }, "save-refused", "the tree holds objects with unknown data" },
		{ { { { "SER" } }, 14 }, "migration-failed", "a migration step failed" },
		{ { { { "SER" } }, 15 }, "layout-mismatch", "layout header, size or fixups do not match the target" },
		{ { { { "SER" } }, 16 }, "unresolved-reference", "a reference names no live object" },
		{ { { { "SER" } }, 17 }, "write-failed", "the archive reported an error while saving" }
	};

	consteval ref<const diag::DiagnosticCodeInfo> ser(const StringView key_) {
		return diag::findCode(table, key_);
	}
}
