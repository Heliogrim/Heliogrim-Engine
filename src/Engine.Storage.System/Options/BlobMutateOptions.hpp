#pragma once

#include <Engine.Storage.Action/Guard/IoMutationGuard.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Blob;
#endif

namespace hg::engine::storage {
	/**/

	using mutate_blob_guard = IoMutationGuard<resource::Blob>;

	using mutate_blob_error = mutate_blob_guard::error_type;
	using async_mutate_blob_error = mutate_blob_guard::error_type;// TODO: Missing std::system_error for future type
}
