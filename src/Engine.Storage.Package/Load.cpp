#include <stddef.h>

#include "Load.hpp"

import std;
import Engine.Crypto;
import Engine.Resource.Archive;
import Engine.Resource.Package;

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	using Kind = PackageError::Kind;

	constexpr u64 headerSize = sizeof(PackageHeader);
	constexpr u64 footerSize = sizeof(PackageFooter);

	[[nodiscard]] PackageError malformed(const u64 offset_, const u64 length_) noexcept;

	[[nodiscard]] bool validEndianness(const PackageEndianness value_) noexcept;

	/**
	 * Empty (0, 0) or a range inside `[headerSize, limit_]` without overflow
	 */
	[[nodiscard]] bool validRange(const u64 offset_, const u64 size_, const u64 limit_) noexcept;

	template <typename LinkerType, typename LinkedType, typename BlobType>
	[[nodiscard]] Result<LinkerType, PackageError> linkIndex(ref<BlobType> blob_, ref<const PackageHeader> header_) noexcept;
}

/**/

Result<PackageHeader, PackageError> engine::storage::package::readHeader(ref<const Blob> blob_) noexcept {

	const auto available = blob_.size();
	// Note: An indeterminate size (`< 0`, e.g. memory blobs) is bounded by probing the last byte below.
	if (available >= 0 && static_cast<u64>(available) < headerSize + footerSize) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(0uLL, headerSize + footerSize));
	}

	/**/

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageHeader));
	archive.seek(0);

	const auto read = blob_.read(streamoff {}, archive.getByteSpan());
	if (read.size() < headerSize) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(0uLL, headerSize));
	}

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

	if (header.magicBytes != PackageMagicBytes || header.magicVersion != PackageMagicVersion[0]) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(0uLL, 7uLL));
	}

	if (not validEndianness(header.endianness)) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, endianness), 1uLL));
	}

	if (header.version > 1uLL) {
		return Result<PackageHeader, PackageError>::unexpected(
			PackageError { Kind::eNewer, offsetof(PackageHeader, version), sizeof(PackageVersion) }
		);
	}

	/**/

	if (available >= 0) {
		const auto total = static_cast<u64>(available);
		if (header.packageSize > total - headerSize - footerSize) {
			return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, packageSize), 8uLL));
		}

	} else {
		const auto last = static_cast<u64>((std::numeric_limits<streamoff>::max)()) - headerSize - footerSize;
		auto probe = _::byte {};
		if (header.packageSize > last ||
			blob_.read(
				static_cast<streamoff>(headerSize + header.packageSize + footerSize - 1uLL),
				std::span<_::byte> { &probe, 1uLL }
			).size() != 1uLL) {
			return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, packageSize), 8uLL));
		}
	}

	const auto limit = headerSize + header.packageSize;
	if (not validRange(header.indexOffset, header.indexSize, limit)) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, indexOffset), 16uLL));
	}

	if (not validRange(header.metaOffset, header.metaSize, limit)) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, metaOffset), 16uLL));
	}

	if (not validRange(header.compDataOffset, header.compDataSize, limit)) {
		return Result<PackageHeader, PackageError>::unexpected(malformed(offsetof(PackageHeader, compDataOffset), 16uLL));
	}

	/**/

	return Result<PackageHeader, PackageError>::expected(::hg::move(header));
}

Result<ReadonlyPackageLinker, PackageError> engine::storage::package::readLinker(
	ref<const Blob> blob_,
	ref<const PackageHeader> header_
) noexcept {
	return linkIndex<ReadonlyPackageLinker, ReadonlyLinkedArchive>(blob_, header_);
}

Result<ReadWritePackageLinker, PackageError> engine::storage::package::readLinker(
	ref<Blob> blob_,
	ref<const PackageHeader> header_
) noexcept {
	return linkIndex<ReadWritePackageLinker, ReadWriteLinkedArchive>(blob_, header_);
}

Result<PackageFooter, PackageError> engine::storage::package::readFooter(
	ref<const Blob> blob_,
	ref<const PackageHeader> header_
) noexcept {

	const auto footerOffset = static_cast<u64>(computeFooterOffset(header_));
	const auto available = blob_.size();
	if (available >= 0 && footerOffset + footerSize > static_cast<u64>(available)) {
		return Result<PackageFooter, PackageError>::unexpected(malformed(footerOffset, footerSize));
	}

	/**/

	auto archive = BufferArchive {};
	archive.resize(sizeof(PackageFooter));
	archive.seek(0);

	const auto read = blob_.read(static_cast<streamoff>(footerOffset), archive.getByteSpan());
	if (read.size() < footerSize) {
		return Result<PackageFooter, PackageError>::unexpected(malformed(footerOffset, footerSize));
	}

	/**/

	auto footer = PackageFooter {};

	archive.serializeBytes(footer.crc32.data(), footer.crc32.size(), ArchiveStreamMode::eLoad);
	archive.serializeBytes(footer.__reserved__.data(), footer.__reserved__.size(), ArchiveStreamMode::eLoad);
	archive.serializeBytes(footer.magicBytes.data(), footer.magicBytes.size(), ArchiveStreamMode::eLoad);
	archive >> footer.magicVersion;
	archive >> footer.endianess;

	/**/

	if (footer.magicBytes != PackageMagicBytes || footer.magicVersion != PackageMagicVersion[0] ||
		not validEndianness(footer.endianess)) {
		return Result<PackageFooter, PackageError>::unexpected(
			malformed(footerOffset + offsetof(PackageFooter, magicBytes), 8uLL)
		);
	}

	return Result<PackageFooter, PackageError>::expected(::hg::move(footer));
}

/**/

Result<u32, PackageError> engine::storage::package::computeChecksum(ref<const Blob> blob_, const streamsize length_) {

	constexpr auto chunkSize = std::size_t { 4096uLL };
	auto buffer = Array<_::byte, chunkSize> {};
	auto checksum = engine::crypto::Crc32 {};

	auto it = streampos {};
	while (it < length_) {

		const auto nextChunkSize = std::min(static_cast<size_t>(length_ - it), chunkSize);
		auto nextChunk = std::span<_::byte> { buffer.data(), nextChunkSize };

		const auto read = blob_.read(it, std::span<_::byte> { nextChunk });
		if (read.size() != nextChunkSize) {
			return Result<u32, PackageError>::unexpected(malformed(static_cast<u64>(it), nextChunkSize));
		}

		checksum.update(std::span<const _::byte> { nextChunk.data(), nextChunkSize });

		it += nextChunkSize;
	}

	return Result<u32, PackageError>::expected(::hg::move(checksum).finalize());
}

u32 engine::storage::package::readChecksum(ref<const PackageFooter> footer_) noexcept {
	// Writer stores `byteswap(u32 crc)` widened to u64: little-endian hosts keep it in the first 4 bytes.
	if constexpr (std::endian::native == std::endian::little) {
		auto stored = u32 {};
		std::memcpy(&stored, footer_.crc32.data(), sizeof(stored));
		return std::byteswap(stored);

	} else {
		auto stored = u64 {};
		std::memcpy(&stored, footer_.crc32.data(), sizeof(stored));
		return static_cast<u32>(stored);
	}
}

Result<void, PackageError> engine::storage::package::verifyChecksum(
	ref<const Blob> blob_,
	ref<const PackageHeader> header_,
	ref<const PackageFooter> footer_
) {
	const auto footerOffset = computeFooterOffset(header_);
	const auto computed = computeChecksum(blob_, footerOffset);
	if (not computed.has_value()) {
		return Result<void, PackageError>::unexpected(computed.error());
	}

	if (computed.value() != readChecksum(footer_)) {
		return Result<void, PackageError>::unexpected(
			PackageError { Kind::eChecksum, static_cast<u64>(footerOffset), footerSize }
		);
	}
	return {};
}

/**/

namespace {
	PackageError malformed(const u64 offset_, const u64 length_) noexcept {
		return PackageError { Kind::eMalformed, offset_, length_ };
	}

	bool validEndianness(const PackageEndianness value_) noexcept {
		return value_ == PackageEndianness::eBigEndian || value_ == PackageEndianness::eLittleEndian;
	}

	bool validRange(const u64 offset_, const u64 size_, const u64 limit_) noexcept {
		if (offset_ == 0uLL && size_ == 0uLL) {
			return true;
		}
		return offset_ >= headerSize && offset_ <= limit_ && size_ <= limit_ - offset_;
	}

	template <typename LinkerType, typename LinkedType, typename BlobType>
	Result<LinkerType, PackageError> linkIndex(ref<BlobType> blob_, ref<const PackageHeader> header_) noexcept {

		const auto indexSize = header_.indexSize;
		if (indexSize % sizeof(Indexed) != 0uLL) {
			return Result<LinkerType, PackageError>::unexpected(malformed(offsetof(PackageHeader, indexSize), 8uLL));
		}

		const auto contentSize = static_cast<u64>(computeContentSize(header_));
		const auto indexCount = indexSize / sizeof(Indexed);

		/**/

		auto index = Vector<Indexed> {};
		static_assert(sizeof(decltype(index)::value_type) == sizeof(Indexed));
		static_assert(alignof(decltype(index)::value_type) == alignof(Indexed));

		// Note: `indexSize` is already bound by the footer offset, which is bound by the blob size.
		index.resize(indexCount);
		if (indexSize > 0uLL) {
			const auto read = blob_.read(static_cast<streamoff>(header_.indexOffset), index.getByteSpan());
			if (read.size() < indexSize) {
				return Result<LinkerType, PackageError>::unexpected(malformed(header_.indexOffset, indexSize));
			}
		}

		/**/

		auto linked = typename LinkerType::link_container_type {};

		for (auto i = 0uLL; i < indexCount; ++i) {

			const auto& indexed = index[i];
			const auto entryOffset = header_.indexOffset + i * sizeof(Indexed);

			const auto offset = indexed.data.offset;
			const auto size = indexed.data.size;
			if (offset < 0 || size < 0 || static_cast<u64>(offset) > contentSize ||
				static_cast<u64>(size) > contentSize - static_cast<u64>(offset)) {
				return Result<LinkerType, PackageError>::unexpected(malformed(entryOffset, sizeof(Indexed)));
			}

			const auto [_, success] = linked.emplace(indexed.header.guid, LinkedType { indexed.data, indexed.header });
			if (not success) {
				return Result<LinkerType, PackageError>::unexpected(malformed(entryOffset, sizeof(Indexed)));
			}
		}

		/**/

		return Result<LinkerType, PackageError>::expected(LinkerType { blob_, ::hg::move(linked) });
	}
}
