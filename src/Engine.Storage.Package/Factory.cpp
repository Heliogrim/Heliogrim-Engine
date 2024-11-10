#include "Factory.hpp"

#include <Engine.Resource.Package/Attribute/MagicBytes.hpp>
#include <Engine.Resource.Package/Attribute/PackageGuid.hpp>

#include "Load.hpp"

using namespace hg::engine::storage::package;
using namespace hg::engine::resource;
using namespace hg;

PackageHeader engine::storage::package::make_package_header() noexcept {

	auto packGuid = PackageGuid {};
	GuidGenerate(packGuid);

	Array<_::byte, 16> netGuid {};
	packGuid.hton(netGuid);

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

ReadonlyPackage engine::storage::package::reconstruct_readonly_package(ref<const Blob> blob_) {

	auto maybeHeader = readHeader(blob_);
	::hg::assertrt(maybeHeader.has_value());

	auto maybeLinker = readLinker(blob_, maybeHeader.value());
	::hg::assertrt(maybeLinker.has_value());

	auto maybeFooter = readFooter(blob_);
	::hg::assertrt(maybeFooter.has_value());

	/**/

	return ReadonlyPackage {
		::hg::move(maybeHeader).value(),
		::hg::move(maybeLinker).value(),
		::hg::move(maybeFooter).value()
	};
}

ReadWritePackage engine::storage::package::reconstruct_read_write_package(ref<Blob> blob_) {

	auto maybeHeader = readHeader(blob_);
	::hg::assertrt(maybeHeader.has_value());

	auto maybeLinker = readLinker(blob_, maybeHeader.value());
	::hg::assertrt(maybeLinker.has_value());

	auto maybeFooter = readFooter(blob_);
	::hg::assertrt(maybeFooter.has_value());

	/**/

	return ReadWritePackage {
		::hg::move(maybeHeader).value(),
		::hg::move(maybeLinker).value(),
		::hg::move(maybeFooter).value()
	};
}
