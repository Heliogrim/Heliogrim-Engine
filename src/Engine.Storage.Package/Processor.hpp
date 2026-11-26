#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.IO.Dispatcher;
import Engine.Resource.Package;
#endif

namespace hg::engine::storage::package {
	/**/

	struct CommitStats {
		u64 oldSize;
		u64 newSize;
		u64 written;
	};

	/**
	 * Splits `[offset_, offset_ + size_)` into contiguous windows of at most 1 MiB (offset, size).
	 */
	[[nodiscard]] Vector<std::pair<u64, u64>> copyWindows(u64 offset_, u64 size_);

	/**
	 * Commits all pending changes by writing the complete new package into a replacement blob and swapping it in.
	 *
	 * On success the file holds the new package and header, footer and linker describe it; a file blob is spent after the swap, so reopen the package by path for further commits.
	 * A flush failure after the swap (`replaced`) counts as swapped.
	 * On failure blob, header, footer and linker are unchanged, so the commit may be retried.
	 */
	[[nodiscard]] Result<void, io::CommitError> commit_package_changes(
		_Inout_ ref<resource::ReadWritePackage> package_,
		_Out_opt_ ptr<CommitStats> stats_ = nullptr
	);
}
