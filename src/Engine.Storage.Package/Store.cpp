#include "Store.hpp"

#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Package/Utils.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

bool engine::storage::package::writeHeader(cref<PackageHeader> header_, ref<Blob> dst_) {

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageHeader));
	archive.seek(0);

	/**/

	archive.serializeBytes(const_cast<ref<PackageHeader>>(header_).magicBytes.data(), header_.magicBytes.size(), ArchiveStreamMode::eStore);
	archive << header_.magicVersion;
	archive << header_.endianness;

	archive << header_.version;
	archive.serializeBytes(const_cast<ref<PackageHeader>>(header_).guid.data(), header_.guid.size(), ArchiveStreamMode::eStore);
	archive << header_.compression;

	archive << header_.__reserved__;

	archive << header_.packageSize;
	archive << header_.inflatedSize;

	archive << header_.indexOffset;
	archive << header_.indexSize;
	archive << header_.metaOffset;
	archive << header_.metaSize;
	archive << header_.compDataOffset;
	archive << header_.compDataSize;

	/**/

	dst_.write(streamoff {}, archive.getByteSpan());

	/**/

	return true;
}

bool engine::storage::package::writeLinker(cref<PackageHeader> header_, ref<ReadWritePackageLinker> linker_, ref<Blob> dst_) {

	::hg::assertrt(header_.compression == PackageCompression::eNone);

	/**/

	const auto indexSize = sizeof(Indexed) * linker_.count();
	::hg::assertrt(indexSize == header_.indexSize);

	auto index = BufferArchive {};
	index.reserve(indexSize);

	for (const auto& linked : linker_) {

		::hg::assertrt(linked.second.changes.empty());

		index << linked.second.data.offset;
		index << linked.second.data.size;
		index << linked.second.header.type;
		index.serializeBytes(
			const_cast<_::byte*>(linked.second.header.guid.data()),
			linked.second.header.guid.size(),
			ArchiveStreamMode::eStore
		);
	}

	/**/

	// Attention: We just assume that we pre-calculated the index offset while updating the data-section
	const auto indexOffset = static_cast<streamoff>(header_.indexOffset);
	dst_.write(indexOffset, index.getByteSpan());

	/**/

	return true;
}

bool engine::storage::package::writeFooter(cref<PackageHeader> header_, cref<PackageFooter> footer_, ref<Blob> dst_) {

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageFooter));
	archive.seek(0);

	/**/

	archive.serializeBytes(
		const_cast<ref<PackageFooter>>(footer_).crc32.data(),
		footer_.crc32.size(),
		ArchiveStreamMode::eStore
	);
	archive.serializeBytes(
		const_cast<ref<PackageFooter>>(footer_).__reserved__.data(),
		footer_.__reserved__.size(),
		ArchiveStreamMode::eStore
	);
	archive.serializeBytes(
		const_cast<ref<PackageFooter>>(footer_).magicBytes.data(),
		footer_.magicBytes.size(),
		ArchiveStreamMode::eStore
	);
	archive << footer_.magicVersion;
	archive << footer_.endianess;

	/**/

	const auto footerOffset = computeFooterOffset(header_);
	dst_.write(footerOffset, archive.getByteSpan());

	/**/

	return true;
}
