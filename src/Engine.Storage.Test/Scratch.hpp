#pragma once

#include <filesystem>

namespace StorageModule {
	/**
	 * The only place storage tests write to: `<temp>/heliogrim-test/storage/<pid>`.
	 *
	 * The run removes its own root on exit. At start and on exit it also removes every sibling root whose process
	 * is gone, so files left by a crashed run or a death-test child are removed by the next run at the latest.
	 */
	[[nodiscard]] const std::filesystem::path& scratchRoot();

	/** Removes every process root next to `scratchRoot()` whose process is gone. Runs at start and exit of the run. */
	void sweepDeadScratch();

	/**
	 * A fresh, empty directory below `scratchRoot()` for the running test. It is removed on destruction, so it must
	 * outlive every storage that holds a file inside it.
	 */
	class ScratchDir {
	public:
		ScratchDir();

		ScratchDir(const ScratchDir&) = delete;

		~ScratchDir();

	public:
		ScratchDir& operator=(const ScratchDir&) = delete;

	public:
		std::filesystem::path path;
	};
}
