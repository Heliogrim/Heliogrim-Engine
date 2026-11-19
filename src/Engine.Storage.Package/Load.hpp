#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

#include "PackageError.hpp"

namespace hg::engine::storage::package {
	class PackageLoadPipeline {};

	/**/

	[[nodiscard]] Result<resource::PackageHeader, PackageError> readHeader(_In_ ref<const resource::Blob> blob_) noexcept;

	[[nodiscard]] Result<resource::ReadonlyPackageLinker, PackageError> readLinker(
		_In_ ref<const resource::Blob> blob_,
		_In_ cref<resource::PackageHeader> header_
	) noexcept;

	[[nodiscard]] Result<resource::ReadWritePackageLinker, PackageError> readLinker(
		_In_ ref<resource::Blob> blob_,
		_In_ cref<resource::PackageHeader> header_
	) noexcept;

	[[nodiscard]] Result<resource::PackageFooter, PackageError> readFooter(
		_In_ ref<const resource::Blob> blob_,
		_In_ ref<const resource::PackageHeader> header_
	) noexcept;

	/**/

	/**
	 * Crc32 over `[0, length_)` of the blob
	 */
	[[nodiscard]] Result<u32, PackageError> computeChecksum(
		_In_ ref<const resource::Blob> blob_,
		_In_ const streamsize length_
	);

	/**
	 * The checksum as stored by the writer (bytes 4..7 of the field, big-endian)
	 */
	[[nodiscard]] u32 readChecksum(_In_ ref<const resource::PackageFooter> footer_) noexcept;

	/**
	 * Fails with `eChecksum` when the stored checksum does not match the content
	 */
	[[nodiscard]] Result<void, PackageError> verifyChecksum(
		_In_ ref<const resource::Blob> blob_,
		_In_ ref<const resource::PackageHeader> header_,
		_In_ ref<const resource::PackageFooter> footer_
	);
}
