#pragma once

#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Action/Guard/IoMutationGuard.hpp>

namespace hg::engine::storage {
	/**/

	using mutate_blob_guard = IoMutationGuard<resource::Blob>;

	using mutate_blob_error = mutate_blob_guard::error_type;
	using async_mutate_blob_error = mutate_blob_guard::error_type;// TODO: Missing std::system_error for future type
}
