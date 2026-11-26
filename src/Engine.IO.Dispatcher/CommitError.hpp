#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::io {
	// Win32 error numbers the commit path reacts to; the code stays platform neutral.
	constexpr u32 error_access_denied = 5u;
	constexpr u32 error_invalid_handle = 6u;
	constexpr u32 error_sharing_violation = 32u;
	constexpr u32 error_lock_violation = 33u;
	constexpr u32 error_not_supported = 50u;
	constexpr u32 error_invalid_parameter = 87u;
	constexpr u32 error_write_protect = 19u;
	constexpr u32 error_crc = 23u;
	constexpr u32 error_handle_disk_full = 39u;
	constexpr u32 error_disk_full = 112u;
	constexpr u32 error_io_device = 1117u;

	/**
	 * Why an all-or-nothing commit (temp write, flush, rename) failed; the old content is intact unless `replaced` is set.
	 */
	struct CommitError {
		enum class Kind : u8 {
			eDiskFull,
			eLocked,
			eAccessDenied,
			eReadOnly,
			eUnsupportedVolume,
			eDamagedSource,
			eDeviceError,
			eFlushFailed,
			eUnknown
		};

		// Volume: retrying the same file elsewhere will not help. File: this file (or its state) is the problem.
		enum class Scope : u8 {
			eVolume,
			eFile
		};

		Kind kind = Kind::eUnknown;
		String path {};
		u32 osError = 0u;
		u64 bytesNeeded = 0uLL;
		u32 retriedMs = 0u;
		// The new file already took the target's name (eFlushFailed after the swap); only its durability is unknown.
		bool replaced = false;

		[[nodiscard]] Scope scope() const noexcept;
	};

	[[nodiscard]] CommitError::Kind commitKindOf(const u32 osError_) noexcept;

	[[nodiscard]] StringView kindName(const CommitError::Kind kind_) noexcept;
}
