#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::diag {
	struct DiagnosticCode {
		/* 3 or 4 upper-case letters, zero padded */
		std::array<char, 4> domain;

		/* 1..9999, rendered as 4 digits */
		u16 number;
	};

	struct DiagnosticCodeInfo {
		DiagnosticCode code;

		/* kebab-case name used at call sites */
		StringView key;

		/* one line, shown in the code list */
		StringView summary;
	};

	/**/

	/**
	 * Docs slug of a code, e.g. `asl/0002`.
	 */
	[[nodiscard]] String slugOf(ref<const DiagnosticCodeInfo> info_);

	/**
	 * Display form of a code, e.g. `ASL0042`.
	 */
	[[nodiscard]] String formatCode(ref<const DiagnosticCode> code_);

	/**
	 * Deliberately not constexpr: reaching it inside `findCode` turns a missing key into a compile error.
	 */
	void unregisteredDiagnosticCode();

	template <std::size_t N>
	consteval ref<const DiagnosticCodeInfo> findCode(const DiagnosticCodeInfo (&table_)[N], const StringView key_) {
		const std::span<const DiagnosticCodeInfo> table { table_ };
		const auto found = FindIf(table, [key_](ref<const DiagnosticCodeInfo> info_) { return info_.key == key_; });
		if (found == table.end()) {
			unregisteredDiagnosticCode();
		}
		return *found;
	}
}
