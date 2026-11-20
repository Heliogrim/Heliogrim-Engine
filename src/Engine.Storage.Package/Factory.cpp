#include "Factory.hpp"

#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Attribute/PackageGuid.hpp>

#include "Load.hpp"

using namespace hg::engine::storage::package;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	template <typename PackageType, typename BlobType>
	[[nodiscard]] Result<PackageType, PackageError> reconstruct(ref<BlobType> blob_);

	template <class PackageType_>
	[[nodiscard]] Result<PackageType_, engine::diag::Diagnostics> reject(
		ref<const PackageError> error_,
		mref<String> blobName_,
		ref<engine::diag::DiagnosticSink> sink_
	);
}

/**/

PackageHeader engine::storage::package::make_package_header() noexcept {

	auto packGuid = PackageGuid {};
	GuidGenerate(packGuid);

	Array<_::byte, 16> netGuid {};
	packGuid.hostToNetwork(netGuid);

	return PackageHeader {
		.magicBytes = PackageMagicBytes,
		.magicVersion = PackageMagicVersion[0],
		.endianness = PackageEndianness::eBigEndian,
		.version = 0x1,
		// TODO: Check whether this is correct !?!
		.guid = netGuid,
		.compression = PackageCompression::eNone,
		.__reserved__ = {},
		.packageSize = 0uLL,
		.inflatedSize = 0uLL,
		.indexOffset = 0uLL,
		.indexSize = 0uLL,
		.metaOffset = 0uLL,
		.metaSize = 0uLL,
		.compDataOffset = 0uLL,
		.compDataSize = 0uLL
	};
}

PackageFooter engine::storage::package::make_package_footer() noexcept {
	return PackageFooter {
		.crc32 = {},
		.__reserved__ = {},
		.magicBytes = PackageMagicBytes,
		.magicVersion = PackageMagicVersion[0],
		.endianess = PackageEndianness::eBigEndian
	};
}

ReadonlyPackage engine::storage::package::make_readonly_package(ref<const Blob> blob_) noexcept {

	auto header = make_package_header();
	auto linker = ReadonlyPackageLinker { blob_, {} };
	auto footer = make_package_footer();

	/**/

	return ReadonlyPackage { ::hg::move(header), ::hg::move(linker), ::hg::move(footer) };
}

ReadWritePackage engine::storage::package::make_read_write_package(ref<Blob> blob_) noexcept {

	auto header = make_package_header();
	auto linker = ReadWritePackageLinker { blob_ };
	auto footer = make_package_footer();

	/**/

	return ReadWritePackage { ::hg::move(header), ::hg::move(linker), ::hg::move(footer) };
}

Result<ReadonlyPackage, PackageError> engine::storage::package::reconstruct_readonly_package(ref<const Blob> blob_) {
	return reconstruct<ReadonlyPackage>(blob_);
}

Result<ReadWritePackage, PackageError> engine::storage::package::reconstruct_read_write_package(ref<Blob> blob_) {
	return reconstruct<ReadWritePackage>(blob_);
}

Result<ReadonlyPackage, engine::diag::Diagnostics> engine::storage::package::open_readonly_package(
	ref<const Blob> blob_,
	String blobName_,
	ref<engine::diag::DiagnosticSink> sink_
) {
	auto opened = reconstruct_readonly_package(blob_);
	if (not opened.has_value()) {
		return reject<ReadonlyPackage>(opened.error(), ::hg::move(blobName_), sink_);
	}

	return Result<ReadonlyPackage, engine::diag::Diagnostics>::expected(::hg::move(opened).value());
}

Result<ReadWritePackage, engine::diag::Diagnostics> engine::storage::package::open_read_write_package(
	ref<Blob> blob_,
	String blobName_,
	ref<engine::diag::DiagnosticSink> sink_
) {
	auto opened = reconstruct_read_write_package(blob_);
	if (not opened.has_value()) {
		return reject<ReadWritePackage>(opened.error(), ::hg::move(blobName_), sink_);
	}

	return Result<ReadWritePackage, engine::diag::Diagnostics>::expected(::hg::move(opened).value());
}

/**/

namespace {
	template <typename PackageType, typename BlobType>
	Result<PackageType, PackageError> reconstruct(ref<BlobType> blob_) {

		auto header = readHeader(blob_);
		if (not header.has_value()) {
			return Result<PackageType, PackageError>::unexpected(header.error());
		}

		auto linker = readLinker(blob_, header.value());
		if (not linker.has_value()) {
			return Result<PackageType, PackageError>::unexpected(linker.error());
		}

		auto footer = readFooter(blob_, header.value());
		if (not footer.has_value()) {
			return Result<PackageType, PackageError>::unexpected(footer.error());
		}

		// Note: Structure first, the checksum only vouches for a structurally sound package.
		auto verified = verifyChecksum(blob_, header.value(), footer.value());
		if (not verified.has_value()) {
			return Result<PackageType, PackageError>::unexpected(verified.error());
		}

		return Result<PackageType, PackageError>::expected(
			PackageType { ::hg::move(header).value(), ::hg::move(linker).value(), ::hg::move(footer).value() }
		);
	}

	template <class PackageType_>
	Result<PackageType_, engine::diag::Diagnostics> reject(
		ref<const PackageError> error_,
		mref<String> blobName_,
		ref<engine::diag::DiagnosticSink> sink_
	) {
		auto diagnostic = toDiagnostic(error_, ::hg::move(blobName_));
		auto copy = diagnostic;
		sink_.emit(::hg::move(diagnostic));

		auto out = engine::diag::Diagnostics {};
		out.push_back(::hg::move(copy));
		return Result<PackageType_, engine::diag::Diagnostics>::unexpected(::hg::move(out));
	}
}
