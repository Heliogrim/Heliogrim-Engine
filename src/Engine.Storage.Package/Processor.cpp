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

[[nodiscard]] constexpr static Vector<std::pair<streamoff, size_t>> sortedIndices(
	_In_ ref<const Vector<ArchiveDelta>> changeList_
) noexcept;

/**/

// TODO: !Important! Rewrite

Result<void, std::runtime_error> engine::storage::package::commit_archive_changes(
	ref<ReadWritePackageLinker> linker_
) {

	auto& blob = linker_.getStorage();

	auto prevSize = streamsize {};
	auto nextSize = streamsize {};
	auto peekSize = streamsize {};

	for (auto& linked : linker_) {
		prevSize += linked.second.data.size;
		nextSize += linked.second.data.size;
		peekSize += linked.second.data.size;

		bool extends = false;

		for (const auto& change : linked.second.changes) {
			change.apply(
				Overloaded {
					[&nextSize, &peekSize, &linked](ref<const ArchiveDeltaAdd> add_) {
						::hg::assertrt(linked.second.data.size == 0 && add_.sizedData.totalSize() >= 0uL);
						nextSize += add_.sizedData.totalSize();
						peekSize += add_.sizedData.totalSize();
					},
					[&nextSize, &peekSize, &extends](ref<const ArchiveDeltaReplace> replace_) {

						const auto insertSize = replace_.withSizedData.totalSize();
						::hg::assertrt(insertSize >= 0uL);

						const auto sizeDiff = insertSize - replace_.byteCount;
						if (sizeDiff > 0) {
							peekSize += sizeDiff;
							extends = true;
						}

						nextSize += sizeDiff;
					},
					[&nextSize](ref<const ArchiveDeltaDrop> drop_) {
						nextSize -= drop_.byteCount;
					}
				}
			);
		}

		if (extends) {
			// Note: When extended, we will duplicate/copy the original sequence into the scratch-tail.
			peekSize += linked.second.data.size;
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
		const auto accessIndices = sortedIndices(archiveChanges);
		auto& archiveHeader = batchChange->header;

		const auto& originSpan = batchChange->data;
		auto workSpan = originSpan;
		auto lastOpEnd = 0LL;

		for (const auto [_, accessIndex] : accessIndices) {
			::hg::move(archiveChanges[accessIndex]).apply(
				Overloaded {
					[&workSpan, &lastOpEnd, &archiveHeader, &scratchTail, &scratchPad](ArchiveDeltaAdd& add_) {

						::hg::assertrt(add_.where == 0 && lastOpEnd == 0);
						const auto insertSize = add_.sizedData.totalSize();

						archiveHeader.type = add_.sizedData.type();
						workSpan.offset = scratchTail;
						workSpan.size = insertSize;

						add_.sizedData.seek(0);
						auto dst = std::span { scratchPad.begin() + scratchTail, static_cast<size_t>(insertSize) };

						add_.sizedData.serializeBytes(dst, ArchiveStreamMode::eLoad);
						scratchTail += insertSize;
					},
					[&originSpan, &workSpan, &lastOpEnd, &archiveHeader, &scratchTail, &scratchPad](ArchiveDeltaReplace& replace_) {

						::hg::assertrt(replace_.where >= lastOpEnd);
						archiveHeader.type = replace_.withSizedData.type();

						const auto insertSize = replace_.withSizedData.totalSize();
						const auto sizeDiff = insertSize - replace_.byteCount;
						/* Note: Normally we would need to track/accumulate this, but because of sorting we can compute */
						const auto drift = workSpan.size - originSpan.size;
						/**
						 *	work ( size = 100 ) | origin ( size = 100 )
						 *		insert ( size = 20 ) + [work - origin] => false
						 *	work ( size = 100 ) | origin ( size = 100 )
						 *		insert ( size = -20 ) + [work - origin] => true
						 *	work ( size = 80 ) | origin ( size = 100 )
						 *		insert ( size = 20 ) + [work - origin] => true
						 *	work ( size = 80 ) | origin ( size = 100 )
						 *		insert ( size = -20 ) + [work - origin] => true
						 *	work ( size = 80 ) | origin ( size = 100 )
						 *		insert ( size = 30 ) + [work - origin] => false
						 */
						const auto fitInPlace = sizeDiff + (workSpan.size - originSpan.size) <= 0;
						const auto inScratchTail = workSpan.offset > originSpan.offset;

						if (fitInPlace) {

							const auto driftedWhere = replace_.where + drift;
							const auto driftedPrevEnd = driftedWhere + replace_.byteCount;
							const auto tailSize = workSpan.size - driftedPrevEnd;
							if (tailSize > 0) {
								auto tailDst = std::span {
									scratchPad.begin() + workSpan.offset + driftedWhere + insertSize, static_cast<size_t>(tailSize)
								};
								memcpy(tailDst.data(), scratchPad.data() + driftedPrevEnd, tailSize);
							}

							auto insertDst = std::span {
								scratchPad.begin() + workSpan.offset + driftedWhere, static_cast<size_t>(insertSize)
							};
							replace_.withSizedData.seek(0);
							replace_.withSizedData.serializeBytes(insertDst, ArchiveStreamMode::eLoad);

							if (inScratchTail) {
								scratchTail += sizeDiff;
							}
							workSpan.size += sizeDiff;
							lastOpEnd = replace_.where + replace_.byteCount;
							return;
						}

						/**/

						if (not inScratchTail) {

							const auto driftedWhere = replace_.where + drift;

							const auto headSize = driftedWhere;
							if (headSize > 0) {
								auto headDst = std::span { scratchPad.begin() + scratchTail, static_cast<size_t>(headSize) };
								memcpy(headDst.data(), scratchPad.data() + originSpan.offset, headSize);
							}

							auto insertDst = std::span { scratchPad.begin() + scratchTail + driftedWhere, static_cast<size_t>(insertSize) };
							replace_.withSizedData.seek(0);
							replace_.withSizedData.serializeBytes(insertDst, ArchiveStreamMode::eLoad);

							const auto driftedPrevEnd = driftedWhere + replace_.byteCount;
							const auto tailSize = workSpan.size - driftedPrevEnd;
							if (tailSize > 0) {
								auto tailDst = std::span {
									scratchPad.begin() + scratchTail + driftedWhere + insertSize, static_cast<size_t>(tailSize)
								};
								memcpy(tailDst.data(), scratchPad.data() + originSpan.offset + driftedPrevEnd, tailSize);
							}

							workSpan.offset = scratchTail;
							workSpan.size += sizeDiff;
							scratchTail += workSpan.size;
							lastOpEnd = replace_.where + replace_.byteCount;
							return;
						}

						/**/

						const auto driftedWhere = replace_.where + drift;
						const auto driftedPrevEnd = driftedWhere + replace_.byteCount;
						const auto tailSize = workSpan.size - driftedPrevEnd;
						if (tailSize > 0) {
							auto tailDst = std::span {
								scratchPad.begin() + workSpan.offset + driftedPrevEnd + sizeDiff, static_cast<size_t>(tailSize)
							};
							std::memmove(tailDst.data(), scratchPad.data() + workSpan.offset + driftedPrevEnd, tailSize);
						}

						auto insertDst = std::span {
							scratchPad.begin() + workSpan.offset + driftedWhere, static_cast<size_t>(insertSize)
						};
						replace_.withSizedData.seek(0);
						replace_.withSizedData.serializeBytes(insertDst, ArchiveStreamMode::eLoad);

						workSpan.size += sizeDiff;
						scratchTail += sizeDiff;
						lastOpEnd = replace_.where + replace_.byteCount;
					},
					[&workSpan, &archiveHeader](ArchiveDeltaDrop& drop_) {

						::hg::assertrt(drop_.where == 0 && drop_.byteCount == workSpan.size);

						archiveHeader.guid = ArchiveGuid {};
						workSpan.offset = streamoff {};
						workSpan.size = streamsize {};
					}
				}
			);
		}

		/**/

		archiveChanges.clear();
		batchChange->data = workSpan;
	}

	/**/

	constexpr auto nullGuid = ArchiveGuid {};

	auto sweeping = Vector<ArchiveGuid> {};
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

	constexpr auto chunkSize = std::size_t { 4096uLL };
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
constexpr Vector<std::pair<streamoff, size_t>> sortedIndices(ref<const Vector<ArchiveDelta>> changeList_) noexcept {
	auto result = Vector<std::pair<streamoff, size_t>> {};
	result.reserve(changeList_.size());
	for (auto index = 0; index < changeList_.size(); ++index) {
		const auto offset = changeList_[index].apply(
			Overloaded {
				[](ref<const ArchiveDeltaAdd> delta_) { return delta_.where; },
				[](ref<const ArchiveDeltaReplace> delta_) { return delta_.where; },
				[](ref<const ArchiveDeltaDrop> delta_) { return delta_.where; }
			}
		);
		result.insert(std::ranges::lower_bound(result, offset, {}, &std::pair<streamoff, size_t>::first), std::pair { offset, index });
	}
	return result;
}
