#include "Processor.hpp"

#include <cstring>

#include "Load.hpp"

import Engine.Common;
import Engine.Crypto;
import Engine.IO.Dispatcher;
import Engine.Resource.Archive;
import Engine.Resource.Package;

using namespace hg::engine::storage::package;
using namespace hg::engine::resource::package;
using namespace hg::engine::resource;
using namespace hg;

/**/

namespace {
	using Outcome = Result<void, engine::io::CommitError>;
	using Kind = engine::io::CommitError::Kind;

	constexpr u64 window = 1uLL << 20uLL;
	constexpr u64 headerSize = sizeof(PackageHeader);
	constexpr u64 footerSize = sizeof(PackageFooter);

	[[nodiscard]] Outcome fail(const Kind kind_, const u32 osError_ = 0u) {
		return Outcome::unexpected(engine::io::CommitError { .kind = kind_, .osError = osError_ });
	}

	[[nodiscard]] Optional<u64> checkedAdd(const u64 lhs_, const u64 rhs_) noexcept {
		if (lhs_ > std::numeric_limits<u64>::max() - rhs_) { return Optional<u64> {}; }
		return lhs_ + rhs_;
	}

	// Fills stats_ as far as the commit got: oldSize always, newSize once planned, written up to a failure.
	[[nodiscard]] Outcome commitInto(ref<ReadWritePackage> package_, ref<CommitStats> stats_);

	/**/

	// A piece of an archive in the new layout: a range of the old file, or the whole data of a delta archive.
	struct Segment {
		ptr<Archive> delta;
		u64 oldOffset;
		u64 size;
	};

	struct Op {
		u64 where;
		u64 byteCount;
		ptr<Archive> data;
	};

	struct ArchivePlan {
		ptr<ReadWriteLinkedArchive> linked;
		ArchiveGuid guid;
		bool dropped;
		ArchiveType type;
		u64 newOffset;
		u64 newSize;
		Vector<Segment> segments;
	};

	/**/

	class Source {
	public:
		Source(ref<const Blob> blob_, const bool crcOn_) :
			_blob(blob_),
			_crcOn(crcOn_),
			_buffer(window) {}

	private:
		ref<const Blob> _blob;
		bool _crcOn;
		u64 _cursor = 0uLL;
		engine::crypto::Crc32 _crc {};
		Vector<_::byte> _buffer;

	public:
		// Feeds the old checksum with the bytes nobody copies, so the source is read exactly once.
		[[nodiscard]] Outcome skipTo(const u64 pos_) {
			if (not _crcOn) { return {}; }
			if (pos_ <= _cursor) { return {}; }
			return this->pass(_cursor, pos_ - _cursor, [](auto) { return Outcome {}; });
		}

		template <typename Fn_>
		[[nodiscard]] Outcome copy(const u64 offset_, const u64 size_, Fn_&& fn_) {
			if (_crcOn) {
				if (auto skipped = this->skipTo(offset_); not skipped.has_value()) { return skipped; }
				return this->pass(offset_, size_, fn_);
			}
			return this->pass(offset_, size_, fn_);
		}

		[[nodiscard]] u32 checksum() && {
			return ::hg::move(_crc).finalize();
		}

	private:
		template <typename Fn_>
		[[nodiscard]] Outcome pass(const u64 offset_, const u64 size_, Fn_&& fn_) {
			auto at = offset_;
			for (const auto& [_, length] : copyWindows(offset_, size_)) {
				std::span<_::byte> dst { _buffer.data(), static_cast<size_t>(length) };
				const auto got = _blob.read(static_cast<streamoff>(at), ::hg::move(dst));
				if (got.size() != length) { return fail(Kind::eDamagedSource); }
				if (_crcOn) {
					_crc.update(got);
					_cursor = at + length;
				}
				if (auto done = fn_(got); not done.has_value()) { return done; }
				at += length;
			}
			return {};
		}
	};

	class Writer {
	public:
		Writer(ref<Blob> blob_, ref<u64> written_) :
			_blob(blob_),
			written(written_) {}

	private:
		ref<Blob> _blob;
		u64 _pos = 0uLL;

	public:
		ref<u64> written;
		engine::crypto::Crc32 crc {};

	public:
		[[nodiscard]] Outcome write(std::span<_::byte> bytes_, const bool checksummed_ = true) {
			const auto length = bytes_.size();
			auto put = _blob.write(static_cast<streamoff>(_pos), ::hg::move(bytes_));
			if (put.size() != length) {
				const auto os = _blob.lastError();
				return fail(os != 0u ? engine::io::commitKindOf(os) : Kind::eDeviceError, os);
			}
			if (checksummed_) { crc.update(put); }
			_pos += length;
			written += length;
			return {};
		}
	};
}

/**/

Vector<std::pair<u64, u64>> engine::storage::package::copyWindows(u64 offset_, u64 size_) {
	auto result = Vector<std::pair<u64, u64>> {};
	while (size_ > 0uLL) {
		const auto length = std::min(size_, window);
		result.emplace_back(offset_, length);
		offset_ += length;
		size_ -= length;
	}
	return result;
}

Result<void, engine::io::CommitError> engine::storage::package::commit_package_changes(
	ref<resource::ReadWritePackage> package_,
	ptr<CommitStats> stats_
) {
	auto stats = CommitStats {};
	auto result = commitInto(package_, stats);
	if (stats_ != nullptr) { *stats_ = stats; }
	return result;
}

/**/

namespace {
	[[nodiscard]] Outcome commitInto(ref<ReadWritePackage> package_, ref<CommitStats> stats_) {
		auto& header = package_.getHeader();
		auto& footer = package_.getFooter();
		auto& linker = package_.getLinker();
		auto& blob = linker.getStorage();

		// Plan: validate the old layout, compute the new one; nothing is written or changed yet.

		const bool creation = blob.size() == 0;
		u64 oldContent = 0uLL, oldFooterOffset = 0uLL, oldTailOffset = 0uLL, oldTailSize = 0uLL;
		if (not creation) {
			// An index (0, 0) is absent: no content, the index slot sits right after the header.
			const auto noIndex = header.indexOffset == 0uLL && header.indexSize == 0uLL;
			const auto indexOffset = noIndex ? headerSize : header.indexOffset;
			if (indexOffset < headerSize) {
				return fail(Kind::eDamagedSource);
			}

			const auto footerOffset = checkedAdd(headerSize, header.packageSize);
			const auto footerEnd = footerOffset ? checkedAdd(*footerOffset, footerSize) : Optional<u64> {};
			const auto indexEnd = checkedAdd(indexOffset, header.indexSize);
			if (not footerEnd || *footerEnd > static_cast<u64>(std::numeric_limits<s64>::max())) {
				return fail(Kind::eDamagedSource);
			}

			const auto sourceSize = blob.size() < 0 ? *footerEnd : static_cast<u64>(blob.size());
			if (*footerEnd > sourceSize || not indexEnd || *indexEnd > *footerOffset) {
				return fail(Kind::eDamagedSource);
			}

			oldContent = indexOffset - headerSize;
			oldFooterOffset = *footerOffset;
			oldTailOffset = *indexEnd;
			oldTailSize = *footerOffset - *indexEnd;
			stats_.oldSize = *footerEnd;
		}

		auto plans = Vector<ArchivePlan> {};
		auto newContent = 0uLL;
		auto kept = 0uLL;
		for (auto& linked : linker) {
			auto& archive = linked.second;
			const auto oldOffset = archive.data.offset;
			const auto oldSize = archive.data.size;
			if (oldOffset < 0 || oldSize < 0) {
				return fail(Kind::eDamagedSource);
			}

			const auto oldEnd = checkedAdd(static_cast<u64>(oldOffset), static_cast<u64>(oldSize));
			if (not oldEnd || *oldEnd > oldContent) {
				return fail(Kind::eDamagedSource);
			}

			auto plan = ArchivePlan {
				.linked = &archive,
				.guid = linked.first,
				.dropped = false,
				.type = archive.header.type,
				.newOffset = newContent,
				.newSize = 0uLL,
				.segments = {}
			};

			auto ops = Vector<Op> {};
			for (auto& change : archive.changes) {
				auto ok = true;
				change.apply(
					Overloaded {
						[&](ref<const ArchiveDeltaAdd> add_) {
							ok = ok && oldSize == 0 && add_.where == 0;
							ops.emplace_back(0uLL, 0uLL, &add_.sizedData);
						},
						[&](ref<const ArchiveDeltaReplace> replace_) {
							ok = ok && replace_.where >= 0 && replace_.byteCount >= 0;
							ops.emplace_back(
								static_cast<u64>(replace_.where),
								static_cast<u64>(replace_.byteCount),
								&replace_.withSizedData
							);
						},
						[&](ref<const ArchiveDeltaDrop> drop_) {
							ok = ok && drop_.where == 0 && drop_.byteCount == oldSize;
							plan.dropped = true;
						}
					}
				);
				if (not ok) { return fail(Kind::eDamagedSource); }
			}

			if (plan.dropped) {
				plans.emplace_back(::hg::move(plan));
				continue;
			}

			std::ranges::stable_sort(ops, {}, &Op::where);
			const auto base = headerSize + static_cast<u64>(oldOffset);
			auto cursor = 0uLL;
			auto push = [&](const Segment segment_) -> bool {
				if (segment_.size == 0uLL) {
					return true;
				}

				const auto next = checkedAdd(plan.newSize, segment_.size);
				if (not next) {
					return false;
				}

				plan.newSize = *next;
				plan.segments.push_back(segment_);
				return true;
			};
			for (const auto& op : ops) {
				const auto end = checkedAdd(op.where, op.byteCount);
				if (op.where < cursor || not end || *end > static_cast<u64>(oldSize)) {
					return fail(Kind::eDamagedSource);
				}

				const auto inserted = op.data->totalSize();
				if (
					inserted < 0 ||
					not push({ nullptr, base + cursor, op.where - cursor }) ||
					not push({ op.data, 0uLL, static_cast<u64>(inserted) })
				) {
					return fail(Kind::eDamagedSource);
				}

				plan.type = op.data->type();
				cursor = *end;
			}
			if (not push({ nullptr, base + cursor, static_cast<u64>(oldSize) - cursor })) {
				return fail(Kind::eDamagedSource);
			}

			const auto next = checkedAdd(newContent, plan.newSize);
			if (not next) {
				return fail(Kind::eDamagedSource);
			}

			newContent = *next;
			++kept;
			plans.push_back(::hg::move(plan));
		}

		const auto newIndexSize = kept * sizeof(Indexed);
		const auto newPackage = checkedAdd(newContent, newIndexSize).and_then(
			[&](u64 v_) {
				return checkedAdd(v_, oldTailSize);
			}
		);

		const auto newTotal = newPackage.and_then([](u64 v_) { return checkedAdd(v_, headerSize + footerSize); });
		if (not newTotal || *newTotal > static_cast<u64>(std::numeric_limits<s64>::max())) {
			return fail(Kind::eDamagedSource);
		}

		const auto newTailOffset = headerSize + newContent + newIndexSize;
		stats_.newSize = *newTotal;

		auto newHeader = header;
		newHeader.indexOffset = headerSize + newContent;
		newHeader.indexSize = newIndexSize;
		newHeader.packageSize = *newPackage;
		// Meta and comp data live in the tail, the only region copied verbatim; an empty range just moves to the new tail.
		auto moveIntoTail = [&](ref<u64> offset_, const u64 size_) -> bool {
			if (offset_ < oldTailOffset) {
				offset_ = newTailOffset;
				return size_ == 0uLL;
			}

			const auto moved = checkedAdd(offset_ - oldTailOffset, newTailOffset);
			const auto movedEnd = moved ? checkedAdd(*moved, size_) : Optional<u64> {};
			if (not movedEnd || *movedEnd > newTailOffset + oldTailSize) {
				return false;
			}

			offset_ = *moved;
			return true;
		};
		auto metaOffset = newHeader.metaOffset;
		auto compDataOffset = newHeader.compDataOffset;
		if (not moveIntoTail(metaOffset, newHeader.metaSize) || not moveIntoTail(compDataOffset, newHeader.compDataSize)) {
			return fail(Kind::eDamagedSource);
		}

		newHeader.metaOffset = metaOffset;
		newHeader.compDataOffset = compDataOffset;

		// Copy segments in new order (content, then tail); in old-file order the source is read once, else it is verified up front.
		auto monotonic = true; {
			auto previousEnd = 0uLL;
			auto track = [&](const u64 offset_, const u64 size_) {
				if (size_ == 0uLL) { return; }
				monotonic = monotonic && offset_ >= previousEnd;
				previousEnd = offset_ + size_;
			};
			for (const auto& plan : plans) {
				for (const auto& segment : plan.segments) {
					if (segment.delta == nullptr) { track(segment.oldOffset, segment.size); }
				}
			}
			track(oldTailOffset, oldTailSize);
		}
		// Only copied bytes need the old crc; a header-and-footer stub (BlobPack) has nothing to keep and a blank crc.
		const auto verify = oldContent != 0uLL || oldTailSize != 0uLL;
		if (verify && not monotonic) {
			if (not verifyChecksum(blob, header, footer).has_value()) {
				return fail(Kind::eDamagedSource);
			}
		}

		auto index = Vector<_::byte>(newIndexSize); {
			auto at = 0uLL;
			for (const auto& plan : plans) {
				if (plan.dropped) {
					continue;
				}

				Indexed indexed {};
				indexed.data.offset = static_cast<streamoff>(plan.newOffset);
				indexed.data.size = static_cast<streamsize>(plan.newSize);
				indexed.header.type = plan.type;
				indexed.header.guid = plan.guid;
				std::memcpy(index.data() + at, &indexed, sizeof(Indexed));
				at += sizeof(Indexed);
			}
		}

		/**/

		// Stream

		auto begun = blob.beginReplace(static_cast<streamsize>(*newTotal));
		if (not begun.has_value()) {
			return Outcome::unexpected(::hg::move(begun).error());
		}

		auto replacement = ::hg::move(begun).value();
		auto writer = Writer { *replacement, stats_.written };
		auto source = Source { blob, verify && monotonic };

		Array<_::byte, sizeof(PackageHeader)> headerBytes {};
		std::memcpy(headerBytes.data(), &newHeader, sizeof(PackageHeader));
		if (auto r = writer.write(headerBytes); not r.has_value()) {
			return r;
		}

		auto emit = [&](std::span<_::byte> bytes_) { return writer.write(bytes_); };
		auto copyOld = [&](const u64 offset_, const u64 size_) { return source.copy(offset_, size_, emit); };

		auto buffer = Vector<_::byte> {};
		for (const auto& plan : plans) {
			for (const auto& segment : plan.segments) {
				if (segment.delta == nullptr) {
					if (auto r = copyOld(segment.oldOffset, segment.size); not r.has_value()) { return r; }
					continue;
				}

				segment.delta->seek(0);
				buffer.resize(std::min(segment.size, window));
				for (const auto& [_, length] : copyWindows(0uLL, segment.size)) {
					std::span<_::byte> part { buffer.data(), static_cast<size_t>(length) };
					segment.delta->serializeBytes(part, ArchiveStreamMode::eLoad);
					if (segment.delta->hasError()) {
						return fail(Kind::eDamagedSource);
					}

					if (auto r = writer.write(part); not r.has_value()) {
						return r;
					}
				}
			}
		}

		if (auto r = writer.write(index); not r.has_value()) {
			return r;
		}
		if (auto r = copyOld(oldTailOffset, oldTailSize); not r.has_value()) {
			return r;
		}

		if (verify && monotonic) {
			if (auto r = source.skipTo(oldFooterOffset); not r.has_value()) {
				return r;
			}
			if (::hg::move(source).checksum() != readChecksum(footer)) {
				return fail(Kind::eDamagedSource);
			}
		}

		auto newFooter = footer;
		const auto value = ::hg::move(writer.crc).finalize();
		const u64 stored = std::endian::native == std::endian::little ? static_cast<u64>(std::byteswap(value)) : static_cast<u64>(value);
		std::memcpy(newFooter.crc32.data(), &stored, sizeof(stored));
		Array<_::byte, sizeof(PackageFooter)> footerBytes {};
		std::memcpy(footerBytes.data(), &newFooter, sizeof(PackageFooter));
		if (auto r = writer.write(footerBytes, false); not r.has_value()) {
			return r;
		}

		// Reserved before the swap so nothing after it allocates.
		auto dropped = Vector<ArchiveGuid> {};
		dropped.reserve(plans.size());

		// A flush failure after the swap still leaves the new file in place, so the state below must follow it.
		auto finished = blob.finishReplace(::hg::move(replacement));
		if (not finished.has_value() && not finished.error().replaced) {
			return finished;
		}

		/**/

		// Apply

		header = newHeader;
		footer = newFooter;
		for (auto& plan : plans) {
			if (plan.dropped) {
				dropped.push_back(plan.guid);
				continue;
			}
			plan.linked->data.offset = static_cast<streamoff>(plan.newOffset);
			plan.linked->data.size = static_cast<streamsize>(plan.newSize);
			plan.linked->header.type = plan.type;
			plan.linked->changes.clear();
		}
		for (const auto& guid : dropped) {
			linker.drop(guid);
		}
		return finished;
	}
}
