#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Blob;
import Engine.Storage.Action;
#endif

namespace hg::engine::storage {
	/**/

	using query_blob_guard = IoQueryGuard<const resource::Blob>;

	using query_blob_error = query_blob_guard::error_type;
	using async_query_blob_error = query_blob_guard::error_type;// TODO: Missing std::system_error for future type
}
