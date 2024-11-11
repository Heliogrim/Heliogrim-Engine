#pragma once

#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Storage.Action/Guard/IoMutationGuard.hpp>

namespace hg::engine::storage {
	/**/

	using mutate_package_guard = IoMutationGuard<resource::ReadWritePackage>;

	using mutate_package_error = mutate_package_guard::error_type;
	using async_mutate_package_error = mutate_package_guard::error_type;// TODO: Missing std::system_error for future type
}
