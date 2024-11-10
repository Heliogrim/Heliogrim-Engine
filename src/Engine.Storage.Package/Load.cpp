#include "Load.hpp"

#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

Opt<PackageHeader> engine::storage::package::readHeader(ref<const Blob> blob_) noexcept {

	const auto available = blob_.size();
	if (available <= sizeof(PackageHeader)) {
		return None;
	}

	/**/

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageHeader));
	archive.seek(0);

	blob_.read(streamoff {}, archive.getByteSpan());

	/**/

	auto header = PackageHeader {};

	archive.serializeBytes(header.magicBytes.data(), header.magicBytes.size(), ArchiveStreamMode::eLoad);
	archive >> header.magicVersion;
	archive >> header.endianness;

	archive >> header.version;
	archive.serializeBytes(header.guid.data(), header.guid.size(), ArchiveStreamMode::eLoad);
	archive >> header.compression;

	archive >> header.__reserved__;

	archive >> header.packageSize;
	archive >> header.inflatedSize;

	archive >> header.indexOffset;
	archive >> header.indexSize;
	archive >> header.metaOffset;
	archive >> header.metaSize;
	archive >> header.compDataOffset;
	archive >> header.compDataSize;

	/**/

	return Some(::hg::move(header));
}

Opt<ReadonlyPackageLinker> engine::storage::package::readLinker(
	ref<const Blob> blob_,
	cref<PackageHeader> header_
) noexcept {

	const auto available = blob_.size();
	if (available < sizeof(PackageHeader)) {
		return None;
	}

	/**/

	const auto indexOffset = header_.indexOffset;
	const auto indexSize = header_.indexSize;

	const auto indexCount = indexSize / sizeof(Indexed);

	/**/

	auto index = Vector<Indexed> {};
	static_assert(sizeof(decltype(index)::value_type) == sizeof(Indexed));
	static_assert(alignof(decltype(index)::value_type) == alignof(Indexed));

	index.resize(indexCount);
	blob_.read(static_cast<streamoff>(indexOffset), index.getByteSpan());

	/**/

	auto linked = ReadonlyPackageLinker::link_container_type {};

	for (auto i = 0uLL; i < indexCount; ++i) {

		const auto& indexed = index[i];
		const auto [_, success] = linked.emplace(
			static_cast<Guid>(indexed.header.guid),
			ReadonlyLinkedArchive { indexed.data, indexed.header }
		);
		::hg::assertrt(success);
	}

	/**/

	return Some(ReadonlyPackageLinker { blob_, ::hg::move(linked) });
}

Opt<ReadWritePackageLinker> engine::storage::package::readLinker(
	ref<Blob> blob_,
	cref<PackageHeader> header_
) noexcept {

	const auto available = blob_.size();
	if (available < sizeof(PackageHeader)) {
		return None;
	}

	/**/

	const auto indexOffset = header_.indexOffset;
	const auto indexSize = header_.indexSize;

	const auto indexCount = indexSize / sizeof(Indexed);

	/**/

	auto index = Vector<Indexed> {};
	static_assert(sizeof(decltype(index)::value_type) == sizeof(Indexed));
	static_assert(alignof(decltype(index)::value_type) == alignof(Indexed));

	index.resize(indexCount);
	blob_.read(static_cast<streamoff>(indexOffset), index.getByteSpan());

	/**/

	auto linked = ReadWritePackageLinker::link_container_type {};

	for (auto i = 0uLL; i < indexCount; ++i) {

		const auto& indexed = index[i];
		const auto [_, success] = linked.emplace(
			static_cast<Guid>(indexed.header.guid),
			ReadWriteLinkedArchive { ReadonlyLinkedArchive { indexed.data, indexed.header }, {} }
		);
		::hg::assertrt(success);
	}

	/**/

	return Some(ReadWritePackageLinker { blob_, ::hg::move(linked) });
}

Opt<PackageFooter> engine::storage::package::readFooter(ref<const Blob> blob_) noexcept {

	const auto available = blob_.size();
	if (available < sizeof(PackageHeader) + sizeof(PackageFooter)) {
		return None;
	}

	/**/

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageFooter));
	archive.seek(0);

	blob_.read(static_cast<streamoff>(available - sizeof(PackageFooter)), archive.getByteSpan());

	/**/

	auto footer = PackageFooter {};

	archive.serializeBytes(footer.crc32.data(), footer.crc32.size(), ArchiveStreamMode::eLoad);
	archive.serializeBytes(footer.__reserved__.data(), footer.__reserved__.size(), ArchiveStreamMode::eLoad);
	archive.serializeBytes(footer.magicBytes.data(), footer.magicBytes.size(), ArchiveStreamMode::eLoad);
	archive >> footer.magicVersion;
	archive >> footer.endianess;

	/**/

	return Some(::hg::move(footer));
}
