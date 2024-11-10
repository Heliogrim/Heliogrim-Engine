#pragma once

#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>

namespace hg::engine::storage::package {
	/**/

	[[nodiscard]] resource::PackageHeader make_package_header() noexcept;

	[[nodiscard]] resource::PackageFooter make_package_footer() noexcept;

	/**/

	[[nodiscard]] resource::ReadonlyPackage make_readonly_package(_In_ ref<const resource::Blob> blob_) noexcept;

	[[nodiscard]] resource::ReadWritePackage make_read_write_package(_Inout_ ref<resource::Blob> blob_) noexcept;

	[[nodiscard]] resource::ReadonlyPackage reconstruct_readonly_package(_In_ ref<const resource::Blob> blob_);

	[[nodiscard]] resource::ReadWritePackage reconstruct_read_write_package(_Inout_ ref<resource::Blob> blob_);
}
