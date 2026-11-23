#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource.Package;
import Engine.Storage.Action;
#endif

namespace hg::engine::storage {
	/**/

	using query_package_guard = IoQueryGuard<const resource::ReadonlyPackage>;

	using query_package_error = query_package_guard::error_type;
	using async_query_package_error = query_package_guard::error_type;// TODO: Missing std::system_error for future type
}
