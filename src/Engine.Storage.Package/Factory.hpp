#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Diagnostics/Diagnostic.hpp>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>

#include "PackageError.hpp"

namespace hg::engine::storage::package {
	/**/

	[[nodiscard]] resource::PackageHeader make_package_header() noexcept;

	[[nodiscard]] resource::PackageFooter make_package_footer() noexcept;

	/**/

	[[nodiscard]] resource::ReadonlyPackage make_readonly_package(_In_ ref<const resource::Blob> blob_) noexcept;

	[[nodiscard]] resource::ReadWritePackage make_read_write_package(_Inout_ ref<resource::Blob> blob_) noexcept;

	[[nodiscard]] Result<resource::ReadonlyPackage, PackageError> reconstruct_readonly_package(_In_ ref<const resource::Blob> blob_);

	[[nodiscard]] Result<resource::ReadWritePackage, PackageError> reconstruct_read_write_package(_Inout_ ref<resource::Blob> blob_);

	/**
	 * Package-open boundary; a rejected package is emitted to the sink and returned as diagnostics, never as an empty package.
	 */
	[[nodiscard]] Result<resource::ReadonlyPackage, diag::Diagnostics> open_readonly_package(
		_In_ ref<const resource::Blob> blob_,
		_In_ String blobName_,
		_In_ ref<diag::DiagnosticSink> sink_
	);

	[[nodiscard]] Result<resource::ReadWritePackage, diag::Diagnostics> open_read_write_package(
		_Inout_ ref<resource::Blob> blob_,
		_In_ String blobName_,
		_In_ ref<diag::DiagnosticSink> sink_
	);
}
