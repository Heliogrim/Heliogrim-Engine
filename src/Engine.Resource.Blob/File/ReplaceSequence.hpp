#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.IO.Dispatcher;
import std;
#endif

namespace hg::engine::resource {
	/**
	 * Backend steps of a file replace; each returns 0 or the OS error.
	 * `replaceByName` closes the temp and target handles before the one MoveFileExW; `flushTarget` gets true after it. After any swap the target blob is spent.
	 */
	struct ReplaceSteps {
		std::function<u32()> flushTemp;
		std::function<u32()> renameTemp;
		std::function<u32()> replaceByName;
		std::function<u32(bool byName_)> flushTarget;
	};

	// Test seam: while set, the first rename step reports error_not_supported so the by-name fallback runs on any volume.
	inline std::atomic_bool forceByNameReplace { false };

	// Space check shared by beginReplace: no value means enough room for size_ plus margin.
	[[nodiscard]] Opt<io::CommitError> checkReplaceSpace(ref<const std::filesystem::path> target_, const streamsize size_);

	// UTF-8 text of a path, for CommitError.path.
	[[nodiscard]] String pathText(ref<const std::filesystem::path> path_);

	// Flush, rename with retries, flush again; `swapped_` is set once the temp took over the target's name.
	[[nodiscard]] Result<void, io::CommitError> runReplaceSteps(
		ref<const std::filesystem::path> target_,
		ref<const ReplaceSteps> steps_,
		ref<bool> swapped_
	);

	// Opens target_ and flushes it (after a by-name replace there is no live handle to flush).
	[[nodiscard]] u32 flushByName(ref<const std::filesystem::path> target_) noexcept;
}
