#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Package;
#endif

namespace hg::engine::storage::package {
	/**/

	[[nodiscard]] Result<void, std::runtime_error> commit_archive_changes(
		_Inout_ ref<resource::ReadWritePackageLinker> linker_
	);

	[[nodiscard]] Result<void, std::runtime_error> commit_package_changes(
		_Inout_ ref<resource::ReadWritePackage> package_
	);
}
