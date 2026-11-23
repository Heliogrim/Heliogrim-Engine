#pragma once

#include <Engine.Storage.Action/Guard/IoMutationGuard.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Package;
#endif

namespace hg::engine::storage {
	/**/

	using mutate_package_guard = IoMutationGuard<resource::ReadWritePackage>;

	using mutate_package_error = mutate_package_guard::error_type;
	using async_mutate_package_error = mutate_package_guard::error_type;// TODO: Missing std::system_error for future type
}
