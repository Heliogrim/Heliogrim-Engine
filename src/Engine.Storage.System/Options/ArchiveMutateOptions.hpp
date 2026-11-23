#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Archive;
import Engine.Storage.Action;
#endif

namespace hg::engine::storage {
	/**/

	using mutate_archive_guard = IoMutationGuard<resource::Archive>;

	using mutate_archive_error = mutate_archive_guard::error_type;
	using async_mutate_archive_error = mutate_archive_guard::error_type;// TODO: Missing std::system_error for future type
}
