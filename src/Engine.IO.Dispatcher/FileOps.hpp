#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

#include "IoTypes.hpp"

namespace hg::engine::io {
	// Suffix of every commit temp sibling; the sweep and the tests match on it.
	inline constexpr std::wstring_view temp_suffix = L".hgtmp";

	struct TempFile {
		ptr<void> handle;
		std::filesystem::path path;
	};

	// Creates `<target>.<8 hex>.hgtmp` next to target_ (CREATE_NEW, share read + delete); flags_ are extra CreateFileW flags.
	[[nodiscard]] Result<TempFile, IoError> createTempSibling(ref<const std::filesystem::path> target_, const u32 flags_);

	// Returns 0 or the OS error; a failed flush is never retried.
	[[nodiscard]] u32 flushFile(const ptr<void> handle_) noexcept;

	// One attempt: POSIX-semantics rename of the open temp handle over target_. Returns 0 or the raw OS error; never calls
	// MoveFileExW. ERROR_INVALID_PARAMETER / ERROR_NOT_SUPPORTED mean the volume lacks POSIX rename: the caller then closes
	// the temp handle and calls replaceByName once. Retry policy belongs to the caller.
	[[nodiscard]] u32 renameOver(const ptr<void> handle_, ref<const std::filesystem::path> target_) noexcept;

	// One MoveFileExW(temp_, target_, REPLACE_EXISTING | WRITE_THROUGH); call it after the temp handle is closed (an open handle always yields a sharing
	// violation). Returns 0 or the OS error.
	[[nodiscard]] u32 replaceByName(ref<const std::filesystem::path> temp_, ref<const std::filesystem::path> target_) noexcept;

	// Removes `<target>.*.hgtmp` siblings whose delete succeeds; locked ones stay.
	void sweepStaleTemps(ref<const std::filesystem::path> target_) noexcept;
}
