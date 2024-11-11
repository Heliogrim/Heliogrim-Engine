#pragma once

#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Storage.Action/Guard/IoQueryGuard.hpp>

namespace hg::engine::storage {
	/**/

	using query_archive_guard = IoQueryGuard<resource::StorageReadonlyArchive>;

	using query_archive_error = query_archive_guard::error_type;
	using async_query_archive_error = query_archive_guard::error_type;// TODO: Missing std::system_error for future type
}
