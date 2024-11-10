#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>

namespace hg::engine::storage::package {
	class PackageLoadPipeline {};

	/**/

	[[nodiscard]] Opt<resource::PackageHeader> readHeader(_In_ ref<const resource::Blob> blob_) noexcept;

	[[nodiscard]] Opt<resource::ReadonlyPackageLinker> readLinker(
		_In_ ref<const resource::Blob> blob_,
		_In_ cref<resource::PackageHeader> header_
	) noexcept;

	[[nodiscard]] Opt<resource::ReadWritePackageLinker> readLinker(
		_In_ ref<resource::Blob> blob_,
		_In_ cref<resource::PackageHeader> header_
	) noexcept;

	[[nodiscard]] Opt<resource::PackageFooter> readFooter(_In_ ref<const resource::Blob> blob_) noexcept;
}
