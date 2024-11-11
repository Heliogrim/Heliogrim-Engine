#pragma once

#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Action/Guard/IoQueryGuard.hpp>

namespace hg::engine::storage {
	/**/

	using query_blob_guard = IoQueryGuard<const resource::Blob>;

	using query_blob_error = query_blob_guard::error_type;
	using async_query_blob_error = query_blob_guard::error_type;// TODO: Missing std::system_error for future type
}
