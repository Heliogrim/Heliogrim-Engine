#pragma once

#include "Engine.Common/Sal.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Resource.Package;
#endif

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
