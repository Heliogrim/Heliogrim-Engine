#pragma once

#include <Engine.Storage.Action/Guard/IoQueryGuard.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Archive;
#endif

namespace hg::engine::storage {
	/**/

	using query_archive_guard = IoQueryGuard<resource::StorageReadonlyArchive>;

	using query_archive_error = query_archive_guard::error_type;
	using async_query_archive_error = query_archive_guard::error_type;// TODO: Missing std::system_error for future type
}
