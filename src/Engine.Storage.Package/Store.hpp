#pragma once

#include "Engine.Common/Sal.hpp"
#include "Engine.Common/Wrapper.hpp"
#include "Engine.Resource.Package/ReadonlyPackage.hpp"
#include "Engine.Resource.Package/ReadWritePackage.hpp"

namespace hg::engine::storage::package {
	class PackageStorePipeline {};

	/**/

	[[nodiscard]] bool writeHeader(
		_In_ cref<resource::PackageHeader> header_,
		_Inout_ ref<resource::Blob> dst_
	);

	[[nodiscard]] bool writeLinker(
		_In_ cref<resource::PackageHeader> header_,
		_Inout_ ref<resource::ReadWritePackageLinker> linker_,
		_Inout_ ref<resource::Blob> dst_
	);

	[[nodiscard]] bool writeFooter(
		_In_ cref<resource::PackageHeader> header_,
		_In_ cref<resource::PackageFooter> footer_,
		_Inout_ ref<resource::Blob> dst_
	);
}
