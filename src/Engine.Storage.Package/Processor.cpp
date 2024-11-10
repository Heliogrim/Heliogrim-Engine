#include "Processor.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Functional/Lambda.hpp>
#include <Engine.Crypto/Checksum/Crc32.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Resource.Package/Utils.hpp>

#include "Store.hpp"

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

/**/

[[nodiscard]] static u32 computeCheckSum(_In_ cref<Blob> blob_, _In_ streamsize length_);

/**/

// TODO: !Important! Rewrite

Result<void, std::runtime_error> engine::storage::package::commit_archive_changes(
	ref<ReadWritePackageLinker> linker_
) {

	auto& blob = linker_.getStorage();

	auto prevSize = streamsize {};
	auto nextSize = streamsize {};
	auto peekSize = streamsize {};

	for (const auto& linked : linker_) {
		prevSize += linked.second.data.size;
		nextSize += linked.second.data.size;
		peekSize += linked.second.data.size;

		for (const auto& change : linked.second.changes) {
			change.apply(
				Overloaded {
					[&nextSize, &peekSize](const ArchiveDeltaAdd& add_) {
						nextSize += add_.size;
						peekSize += add_.size;
					},
					[&nextSize, &peekSize, &linked](const ArchiveDeltaReplace& replace_) {
						if (linked.second.data.size < replace_.size) {
							peekSize += replace_.size;
							nextSize += (replace_.size - linked.second.data.size);

						} else {
							nextSize -= (linked.second.data.size - replace_.size);
						}
					},
					[&nextSize](const ArchiveDeltaDrop& drop_) {
						nextSize -= drop_.size;
					}
				}
			);
		}
	}

	/**/

	auto scratchPad = Vector<_::byte> {};
	scratchPad.resize((std::max)(prevSize, peekSize));

	blob.read(sizeof(PackageHeader), { scratchPad.begin(), static_cast<size_t>(prevSize) });

	/**/

	auto scratchTail = streamoff { prevSize };
	auto batchChangeList = linker_.computeChangeList();

	// Problem: Just consuming the change list won't be sufficient, as we may need to move other archives too, to
	// Problem:		keep the memory dense.
	for (auto&& batchChange : batchChangeList) {

		auto& archiveChanges = batchChange->changes;
		auto& archiveHeader = batchChange->header;
		auto& archiveData = batchChange->data;

		for (auto&& change : archiveChanges) {
			::hg::move(change).apply(
				Overloaded {
					[&archiveData, &scratchTail, &scratchPad](ArchiveDeltaAdd& add_) {

						::hg::assertrt(add_.where == 0);

						archiveData.offset = scratchTail;
						archiveData.size = add_.size;

						add_.data.seek(0);
						auto dst = std::span { scratchPad.begin() + scratchTail, static_cast<size_t>(add_.size) };

						add_.data.serializeBytes(dst, ArchiveStreamMode::eLoad);
						scratchTail += add_.size;
					},
					[&archiveData, &scratchTail, &scratchPad](ArchiveDeltaReplace& replace_) {

						::hg::assertrt(replace_.where == 0);

						if (archiveData.size < replace_.size) {

							archiveData.offset = scratchTail;
							archiveData.size = replace_.size;

							replace_.data.seek(0);
							auto dst = std::span { scratchPad.begin() + scratchTail, static_cast<size_t>(replace_.size) };

							replace_.data.serializeBytes(dst, ArchiveStreamMode::eLoad);
							scratchTail += replace_.size;
							return;
						}

						/**/

						//archiveData.offset = archiveData.offset;
						//archiveData.size = replace_.size;

						replace_.data.seek(0);
						auto dst = std::span { scratchPad.begin() + archiveData.offset, static_cast<size_t>(replace_.size) };

						replace_.data.serializeBytes(dst, ArchiveStreamMode::eLoad);
						archiveData.size = replace_.size;
					},
					[&archiveData, &archiveHeader](ArchiveDeltaDrop& drop_) {

						::hg::assertrt(drop_.where == 0 && drop_.size == archiveData.size);

						archiveHeader.guid = ArchiveGuid {};
						archiveData.offset = streamoff {};
						archiveData.size = streamsize {};
					}
				}
			);
		}

		/**/

		archiveChanges.clear();
	}

	/**/

	constexpr auto nullGuid = ArchiveGuid {};

	auto sweeping = Vector<Guid> {};
	auto blobOffset = streamoff { sizeof(PackageHeader) };

	for (auto& linked : linker_) {

		auto& data = linked.second.data;
		auto& header = linked.second.header;

		if (data.size == 0 && data.offset == 0 && header.guid == nullGuid) {
			sweeping.emplace_back(linked.first);
			continue;
		}

		/**/

		blob.write(blobOffset, { scratchPad.begin() + data.offset, static_cast<size_t>(data.size) });
		data.offset = static_cast<streamoff>(blobOffset - sizeof(PackageHeader));

		blobOffset += data.size;
	}

	for (auto&& erase : sweeping) {
		linker_.drop(::hg::move(erase));
	}

	/**/

	return {};
}

Result<void, std::runtime_error> engine::storage::package::commit_package_changes(ref<resource::ReadWritePackage> package_) {

	auto& header = package_.getHeader();
	auto& blob = package_.getLinker().getStorage();

	// Note: Package :: [Header <-> Content <-> Index <-> Tail <-> Footer]
	// Note: Size(Package) :: Size(Content) + Size(Index) + Size(Meta) + Size(Compress)
	// Note: InflateSize(Package) :: InflateSize(Content) + InflateSize(Index) + Inflate(Meta)

	// Note: Size(Tail+) :: Size(Package) - Size(Content)
	// Note: Size(Tail) :: (Size(Package) - Size(Content)) - Size(Index)

	const auto prevFullSize = sizeof(PackageHeader) + header.packageSize + sizeof(PackageFooter);
	const auto prevIndexSize = header.indexSize;
	const auto prevTailSize = header.packageSize - computeContentSize(header);

	// TODO: This is just for testing purposes. We may include a rt-check later for reasonable sizes
	::hg::assertd(prevTailSize < 4 * 1024 * 1024);

	auto preserve = Vector<_::byte> {};
	preserve.resize(prevTailSize);

	// Note: We copy the old index, which may be required to deal with while error handling
	blob.read(static_cast<streamoff>(header.indexOffset), { preserve.begin(), prevTailSize });

	/**/

	auto& linker = package_.getLinker();
	auto result = commit_archive_changes(linker);
	::hg::assertrt(result.has_value());

	/**/

	const auto nextSectionSize = linker.computeArchiveSize();
	const auto nextIndexSize = linker.computeIndexSize();

	const streamoff nextIndexOffset = static_cast<streamoff>(sizeof(PackageHeader) + nextSectionSize);
	const streamoff nextTailOffset = nextIndexOffset + nextIndexSize;

	const streamsize tailOffsetDiff = nextTailOffset - static_cast<streamsize>(header.indexOffset + header.indexSize);

	/**/

	header.indexOffset = nextIndexOffset;
	header.indexSize = nextIndexSize;

	const auto linkerResult = writeLinker(header, linker, blob);
	::hg::assertrt(linkerResult);

	/**/

	// TODO: Recompute compression data

	header.metaOffset += tailOffsetDiff;
	header.compDataOffset += tailOffsetDiff;

	/**/

	// TODO: Check whether this re-computation is correct when employing meta data and compression.
	header.packageSize = computeContentSize(header) + header.indexSize + header.metaSize + header.compDataSize;

	const auto headerResult = writeHeader(header, blob);
	::hg::assertrt(headerResult);

	/**/

	// TODO: Restore previous tailing data.
	// TODO: Recompute (crc-)signatures and footer data
	auto& footer = package_.getFooter();

	blob.write(
		static_cast<streamoff>(nextTailOffset),
		{ preserve.begin() + static_cast<streamsize>(prevIndexSize), prevTailSize - prevIndexSize }
	);

	if constexpr (std::endian::native == std::endian::little) {
		const auto offset = computeFooterOffset(header);
		auto& dst = *std::bit_cast<ptr<u64>>(footer.crc32.data());
		dst = std::byteswap(computeCheckSum(blob, offset));
	} else {
		const auto offset = computeFooterOffset(header);
		auto& dst = *std::bit_cast<ptr<u64>>(footer.crc32.data());
		dst = computeCheckSum(blob, offset);
	}

	const auto footerResult = writeFooter(header, footer, blob);
	::hg::assertrt(footerResult);

	/**/

	const auto nextEOS = computeFooterOffset(header) + sizeof(PackageFooter);
	if (nextEOS < prevFullSize) {
		if (not blob.trim(static_cast<streamsize>(prevFullSize - nextEOS))) {
			return Unexpected { std::runtime_error("Failed to trim package storage.") };
		}
	}

	/**/

	return {};
}

/**/

u32 computeCheckSum(cref<Blob> blob_, streamsize length_) {

	constexpr auto chunkSize = 4096uLL;
	auto buffer = Array<_::byte, chunkSize> {};
	auto checksum = engine::crypto::Crc32 {};

	auto it = streampos {};
	while (it < length_) {

		const auto nextChunkSize = std::min(static_cast<size_t>(length_ - it), chunkSize);
		auto nextChunk = std::span { buffer.data(), nextChunkSize };

		blob_.read(it, clone(nextChunk));
		checksum.update(nextChunk);

		it += nextChunkSize;
	}

	return ::hg::move(checksum).finalize();
}
