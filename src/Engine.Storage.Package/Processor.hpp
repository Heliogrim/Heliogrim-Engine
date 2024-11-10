#pragma once

#include <stdexcept>
#include <Engine.Common/Expected.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Resource.Package/Linker/ReadWritePackageLinker.hpp>

namespace hg::engine::storage::package {
	/**/

	[[nodiscard]] Result<void, std::runtime_error> commit_archive_changes(
		_Inout_ ref<resource::ReadWritePackageLinker> linker_
	);

	[[nodiscard]] Result<void, std::runtime_error> commit_package_changes(
		_Inout_ ref<resource::ReadWritePackage> package_
	);
}
