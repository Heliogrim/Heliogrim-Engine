#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Storage.Registry/Url/Scheme.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Filesystem;
#endif

namespace hg::engine::storage {
	class StorageSystem;
}

namespace hg::engine::storage {
	/**
	 * Reads the whole file at an absolute path through the storage system.
	 *
	 * @returns None, if the file does not exist or no storage could be obtained for it.
	 */
	[[nodiscard]] Opt<String> readTextViaStorage(
		_In_ ref<StorageSystem> system_,
		_In_ ref<const UrlScheme> scheme_,
		_In_ ref<const fs::Path> absolutePath_
	);
}
