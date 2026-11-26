#include "pch.h"

/**/

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <limits>

import Engine.Common;
import Engine.Async;
import Engine.Resource.Archive;
import Engine.Crypto;
import Engine.Reflect;
import Engine.Resource.Blob;
import Engine.Resource.Package;
import Engine.Diagnostics;
import Engine.IO.Dispatcher;
import Engine.Filesystem;
import Engine.Storage.Package;

using namespace hg::engine::storage::package;
using namespace hg::engine::resource;
using namespace hg;

namespace Commit {
	namespace {
		using Bytes = std::vector<_::byte>;
		using Kind = engine::io::CommitError::Kind;

		constexpr auto headerSize = sizeof(PackageHeader);
		constexpr auto footerSize = sizeof(PackageFooter);

		/* Replacement that fails the k-th write short (OS error 112, disk full). */
		class FaultRep final :
			public InheritMeta<FaultRep, Blob> {
		public:
			FaultRep(Rc<Blob> inner_, ptr<s64> counter_, const s64 failAt_) :
				_inner(std::move(inner_)),
				_counter(counter_),
				_failAt(failAt_) {}

		private:
			Rc<Blob> _inner;
			ptr<s64> _counter;
			s64 _failAt;
			bool _failed = false;

		public:
			std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override {
				return _inner->read(offset_, std::move(dst_));
			}

			std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override {
				if ((*_counter)++ == _failAt) {
					_failed = true;
					return std::move(src_).subspan(0, 0);
				}
				return _inner->write(offset_, std::move(src_));
			}

			bool trim(size_t tailSize_) override { return _inner->trim(tailSize_); }

			u32 lastError() const noexcept override { return _failed ? 112u : 0u; }

			streamsize size() const noexcept override { return _inner->size(); }
		};

		class FaultBlob final :
			public InheritMeta<FaultBlob, Blob> {
		public:
			enum class At { eNone, eBegin, eWrite, eFinish, eFlushAfterSwap };

			FaultBlob(std::span<_::byte> bytes_, const size_t used_) :
				_inner(bytes_, used_) {}

		private:
			ByteSpanBlob _inner;

		public:
			At at = At::eNone;
			Kind kind = Kind::eUnknown;
			s64 writeIndex = 0;
			s64 writes = 0;

			std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override {
				return _inner.read(offset_, std::move(dst_));
			}

			std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override {
				return _inner.write(offset_, std::move(src_));
			}

			bool trim(size_t tailSize_) override { return _inner.trim(tailSize_); }

			streamsize size() const noexcept override { return _inner.size(); }

			Result<Rc<Blob>, engine::io::CommitError> beginReplace(streamsize size_) override {
				writes = 0;
				if (at == At::eBegin) {
					return Result<Rc<Blob>, engine::io::CommitError>::unexpected(engine::io::CommitError { .kind = kind });
				}
				auto begun = _inner.beginReplace(size_);
				if (not begun.has_value()) { return begun; }
				const auto failAt = at == At::eWrite ? writeIndex : s64 { -1 };
				return Rc<Blob> { Rc<FaultRep>::create(std::move(begun).value(), &writes, failAt) };
			}

			Result<void, engine::io::CommitError> finishReplace(mref<Rc<Blob>> replacement_) override {
				if (at == At::eFinish) {
					return Result<void, engine::io::CommitError>::unexpected(engine::io::CommitError { .kind = kind });
				}
				auto finished = _inner.finishReplace(std::move(replacement_));
				if (at == At::eFlushAfterSwap && finished.has_value()) {
					return Result<void, engine::io::CommitError>::unexpected(
						engine::io::CommitError { .kind = Kind::eFlushFailed, .replaced = true }
					);
				}
				return finished;
			}
		};

		/* Real file blob whose replacement fails the k-th write short. */
		class FileFault final :
			public InheritMeta<FileFault, Blob> {
		public:
			FileFault(Rc<Blob> real_, const s64 failAt_) :
				_real(std::move(real_)),
				_failAt(failAt_) {}

		private:
			Rc<Blob> _real;
			s64 _failAt;
			s64 _writes = 0;

		public:
			std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override { return _real->read(offset_, std::move(dst_)); }

			std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override { return _real->write(offset_, std::move(src_)); }

			bool trim(size_t tailSize_) override { return _real->trim(tailSize_); }

			streamsize size() const noexcept override { return _real->size(); }

			Result<Rc<Blob>, engine::io::CommitError> beginReplace(streamsize size_) override {
				auto begun = _real->beginReplace(size_);
				if (not begun.has_value()) { return begun; }
				return Rc<Blob> { Rc<FaultRep>::create(std::move(begun).value(), &_writes, _failAt) };
			}

			Result<void, engine::io::CommitError> finishReplace(mref<Rc<Blob>> replacement_) override { return _real->finishReplace(std::move(replacement_)); }
		};

		struct Keep {
			std::vector<uptr<BufferArchive>> archives;

			ref<Archive> make(const std::string_view text_) {
				auto archive = make_uptr<BufferArchive>();
				archive->reserve(text_.size());
				archive->setType(ArchiveType::eRaw);
				for (const auto& ch : text_) { *archive << ch; }
				return *archives.emplace_back(std::move(archive));
			}
		};

		struct Ids {
			ArchiveGuid a = ArchiveGuid::random();
			ArchiveGuid b = ArchiveGuid::random();
		};

		/* Creates a package holding a = "AAAA", b = "BBBBBB". */
		ReadWritePackage seed(ref<Blob> blob_, ref<const Ids> ids_, ref<Keep> keep_, ptr<const PackageHeader> like_ = nullptr) {
			auto package = make_read_write_package(blob_);
			if (like_ != nullptr) { package.getHeader().guid = like_->guid; }
			EXPECT_TRUE(commit_package_changes(package).has_value());
			for (const auto& [guid, text] : { std::pair { ids_.a, "AAAA" }, std::pair { ids_.b, "BBBBBB" } }) {
				auto& linked = package.getLinker().add({ .type = ArchiveType::eUndefined, .guid = guid });
				linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0, .sizedData = keep_.make(text) });
			}
			EXPECT_TRUE(commit_package_changes(package).has_value());
			return package;
		}

		void replaceA(ref<ReadWritePackage> package_, ref<const Ids> ids_, ref<Keep> keep_) {
			for (auto& linked : package_.getLinker()) {
				if (linked.first == ids_.a) {
					linked.second.changes.emplace_back(
						package::ArchiveDeltaReplace { .where = 1, .byteCount = 2, .withSizedData = keep_.make("xyz") }
					);
				}
			}
		}

		std::map<std::string, std::string> contents(ref<Bytes> raw_, const size_t used_) {
			auto blob = ByteSpanBlob { std::span<_::byte> { raw_.data(), raw_.size() }, used_ };
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(blob, "t.hgpk", sink);
			std::map<std::string, std::string> result;
			EXPECT_TRUE(opened.has_value());
			if (not opened.has_value()) { return result; }
			for (const auto& linked : opened.value().getLinker()) {
				std::string text(static_cast<size_t>(linked.second.data.size), '\0');
				blob.read(
					static_cast<streamoff>(headerSize + linked.second.data.offset),
					std::span<_::byte> { reinterpret_cast<_::byte*>(text.data()), text.size() }
				);
				std::string key;
				for (const auto& b : linked.first) { key += std::to_string(static_cast<unsigned>(b)) + ","; }
				result.emplace(std::move(key), std::move(text));
			}
			return result;
		}

		std::vector<std::string> sortedTexts(ref<Bytes> raw_, const size_t used_) {
			std::vector<std::string> texts;
			for (auto& [_, text] : contents(raw_, used_)) { texts.push_back(text); }
			std::ranges::sort(texts);
			return texts;
		}
	}

	TEST(Commit, EveryStepFailureKeepsOldPackage) {
		struct Point {
			FaultBlob::At at;
			Kind kind;
			s64 index;
		};
		std::vector<Point> points { { FaultBlob::At::eBegin, Kind::eDiskFull, 0 }, { FaultBlob::At::eFinish, Kind::eFlushFailed, 0 }, { FaultBlob::At::eFinish, Kind::eLocked, 0 } };
		for (auto k = 0; k < 64; ++k) { points.push_back({ FaultBlob::At::eWrite, Kind::eDiskFull, k }); }

		auto cleanRuns = 0;
		for (const auto& point : points) {
			Bytes raw(8192uLL);
			FaultBlob blob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
			Ids ids;
			Keep keep;
			auto package = seed(blob, ids, keep);
			const auto before = raw;
			const auto used = static_cast<size_t>(blob.size());

			blob.at = point.at;
			blob.kind = point.kind;
			blob.writeIndex = point.index;
			replaceA(package, ids, keep);
			auto stats = CommitStats { .oldSize = ~0uLL, .newSize = ~0uLL, .written = ~0uLL };
			const auto result = commit_package_changes(package, &stats);

			if (result.has_value()) {
				// Past the last write: the fault never fired, the commit is clean.
				EXPECT_EQ(point.at, FaultBlob::At::eWrite);
				++cleanRuns;
				break;
			}
			EXPECT_EQ(result.error().kind, point.kind);
			EXPECT_EQ(stats.oldSize, used);
			if (point.at == FaultBlob::At::eBegin) { EXPECT_EQ(stats.written, 0uLL); }
			if (point.at == FaultBlob::At::eFinish) { EXPECT_EQ(stats.written, stats.newSize); }
			EXPECT_EQ(raw, before);
			EXPECT_TRUE(package.getLinker().hasChanges());
			EXPECT_EQ(sortedTexts(raw, used), (std::vector<std::string> { "AAAA", "BBBBBB" }));
		}
		EXPECT_EQ(cleanRuns, 1);
	}

	TEST(Commit, RetryAfterFailureMatchesFirstCommit) {
		Ids ids;
		Keep keep;

		Bytes cleanRaw(8192uLL);
		FaultBlob cleanBlob { std::span<_::byte> { cleanRaw.data(), cleanRaw.size() }, 0uLL };
		auto clean = seed(cleanBlob, ids, keep);
		replaceA(clean, ids, keep);
		ASSERT_TRUE(commit_package_changes(clean).has_value());

		Bytes raw(8192uLL);
		FaultBlob blob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
		auto package = seed(blob, ids, keep, &clean.getHeader());
		replaceA(package, ids, keep);
		blob.at = FaultBlob::At::eWrite;
		blob.writeIndex = 1;
		ASSERT_FALSE(commit_package_changes(package).has_value());
		blob.at = FaultBlob::At::eNone;
		ASSERT_TRUE(commit_package_changes(package).has_value());

		ASSERT_EQ(blob.size(), cleanBlob.size());
		const auto size = static_cast<size_t>(blob.size());
		EXPECT_TRUE(std::ranges::equal(std::span { raw.data(), size }, std::span { cleanRaw.data(), size }));
		EXPECT_FALSE(package.getLinker().hasChanges());
	}

	TEST(Commit, FlushFailureAfterSwapFollowsNewFile) {
		Ids ids;
		Keep keep;

		Bytes cleanRaw(8192uLL);
		FaultBlob cleanBlob { std::span<_::byte> { cleanRaw.data(), cleanRaw.size() }, 0uLL };
		auto clean = seed(cleanBlob, ids, keep);
		replaceA(clean, ids, keep);
		ASSERT_TRUE(commit_package_changes(clean).has_value());

		Bytes raw(8192uLL);
		FaultBlob blob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
		auto package = seed(blob, ids, keep, &clean.getHeader());
		replaceA(package, ids, keep);
		blob.at = FaultBlob::At::eFlushAfterSwap;
		const auto res = commit_package_changes(package);
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, Kind::eFlushFailed);
		EXPECT_FALSE(package.getLinker().hasChanges());

		ASSERT_EQ(blob.size(), cleanBlob.size());
		const auto size = static_cast<size_t>(blob.size());
		EXPECT_TRUE(std::ranges::equal(std::span { raw.data(), size }, std::span { cleanRaw.data(), size }));

		// The in-memory state follows the swapped file, so the next commit builds on it.
		blob.at = FaultBlob::At::eNone;
		replaceA(package, ids, keep);
		EXPECT_TRUE(commit_package_changes(package).has_value());
	}

	TEST(Commit, FlippedContentByteIsDamagedSource) {
		Bytes raw(8192uLL);
		FaultBlob blob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
		Ids ids;
		Keep keep;
		auto package = seed(blob, ids, keep);

		raw[headerSize + 5uLL] = static_cast<_::byte>(static_cast<unsigned char>(raw[headerSize + 5uLL]) ^ 0x01);
		const auto before = raw;
		replaceA(package, ids, keep);
		const auto result = commit_package_changes(package);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, Kind::eDamagedSource);
		EXPECT_EQ(raw, before);
	}

	TEST(Commit, ZeroIndexWithMetaIsDamagedNotCreated) {
		// Non-empty source, index (0, 0), meta reaching past the footer: never treated as a fresh package.
		Bytes raw(headerSize + 16uLL + footerSize);
		auto header = make_package_header();
		header.packageSize = 16uLL;
		header.metaOffset = headerSize + 8uLL;
		header.metaSize = 64uLL;
		std::memcpy(raw.data(), &header, headerSize);
		const auto footer = make_package_footer();
		std::memcpy(raw.data() + headerSize + 16uLL, &footer, footerSize);
		const auto before = raw;

		auto blob = ByteSpanBlob { std::span<_::byte> { raw.data(), raw.size() }, raw.size() };
		auto package = make_read_write_package(blob);
		package.getHeader() = header;
		Keep keep;
		auto& linked = package.getLinker().add({ .type = ArchiveType::eUndefined, .guid = ArchiveGuid::random() });
		linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0, .sizedData = keep.make("AAAA") });

		const auto result = commit_package_changes(package);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, Kind::eDamagedSource);
		EXPECT_EQ(raw, before);
	}

	TEST(Commit, PlanRejectsBadLayout) {
		enum class Row {
			eIndexBeforeHeader,
			eIndexOffsetHuge,
			ePackageSizeHuge,
			ePackageSizeOverflow,
			eIndexSizePastFooter,
			eReplaceNegativeWhere,
			eReplaceNegativeCount,
			eReplacePastEnd,
			eDropWrongCount,
			eAddOnNonEmpty,
			eAddWithWhere,
			eOpsOverlap,
		};
		using enum Row;

		for (const auto row : { eIndexBeforeHeader, eIndexOffsetHuge, ePackageSizeHuge, ePackageSizeOverflow, eIndexSizePastFooter, eReplaceNegativeWhere, eReplaceNegativeCount, eReplacePastEnd, eDropWrongCount, eAddOnNonEmpty, eAddWithWhere, eOpsOverlap }) {
			SCOPED_TRACE(static_cast<int>(row));
			Bytes raw(8192uLL);
			FaultBlob blob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
			Ids ids;
			Keep keep;
			auto package = seed(blob, ids, keep);
			const auto before = raw;
			const auto sizeBefore = blob.size();

			auto& header = package.getHeader();
			switch (row) {
				case eIndexBeforeHeader: header.indexOffset = headerSize - 1uLL; break;
				case eIndexOffsetHuge: header.indexOffset = header.packageSize + headerSize + 1uLL; break;
				case ePackageSizeHuge: header.packageSize += 4096uLL; break;
				case ePackageSizeOverflow: header.packageSize = std::numeric_limits<u64>::max() - 8uLL; break;
				case eIndexSizePastFooter: header.indexSize = header.packageSize; break;
				default: break;
			}

			for (auto& linked : package.getLinker()) {
				if (linked.first != ids.a) { continue; }
				auto& changes = linked.second.changes;
				switch (row) {
					case eReplaceNegativeWhere:
						changes.emplace_back(package::ArchiveDeltaReplace { .where = -1, .byteCount = 2, .withSizedData = keep.make("xyz") });
						break;
					case eReplaceNegativeCount:
						changes.emplace_back(package::ArchiveDeltaReplace { .where = 0, .byteCount = -1, .withSizedData = keep.make("xyz") });
						break;
					case eReplacePastEnd:
						changes.emplace_back(package::ArchiveDeltaReplace { .where = 1, .byteCount = 100, .withSizedData = keep.make("xyz") });
						break;
					case eDropWrongCount: changes.emplace_back(package::ArchiveDeltaDrop { .where = 0, .byteCount = 1 }); break;
					case eAddOnNonEmpty: changes.emplace_back(package::ArchiveDeltaAdd { .where = 0, .sizedData = keep.make("zz") }); break;
					case eAddWithWhere: changes.emplace_back(package::ArchiveDeltaAdd { .where = 2, .sizedData = keep.make("zz") }); break;
					case eOpsOverlap:
						changes.emplace_back(package::ArchiveDeltaReplace { .where = 0, .byteCount = 3, .withSizedData = keep.make("xyz") });
						changes.emplace_back(package::ArchiveDeltaReplace { .where = 1, .byteCount = 2, .withSizedData = keep.make("xyz") });
						break;
					default: break;
				}
			}

			const auto result = commit_package_changes(package);
			ASSERT_FALSE(result.has_value());
			EXPECT_EQ(result.error().kind, Kind::eDamagedSource);
			EXPECT_EQ(blob.size(), sizeBefore);
			EXPECT_EQ(raw, before);
		}
	}

	TEST(Commit, FailedFileCommitLeavesNoTemp) {
		const auto dir = std::filesystem::temp_directory_path() / "heliogrim_commit_temp";
		std::filesystem::remove_all(dir);
		std::filesystem::create_directories(dir);
		const auto path = dir / "t.hgpk";

		Ids ids;
		Keep keep;
		Bytes seeded(8192uLL);
		auto seedBlob = ByteSpanBlob { std::span<_::byte> { seeded.data(), seeded.size() }, 0uLL };
		seed(seedBlob, ids, keep);
		const auto seededSize = static_cast<size_t>(seedBlob.size());
		{
			std::ofstream out { path, std::ios::binary | std::ios::trunc };
			out.write(reinterpret_cast<const char*>(seeded.data()), static_cast<std::streamsize>(seededSize));
		}

		{
			LfsSyncBlob file { fs::Path { path } };
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(file, "t.hgpk", sink);
			ASSERT_TRUE(opened.has_value());
			{
				// Damage the content after open: the commit's crc check fails only after the replacement was streamed.
				std::fstream io { path, std::ios::binary | std::ios::in | std::ios::out };
				io.seekp(static_cast<std::streamoff>(headerSize + 5uLL));
				io.put(static_cast<char>(static_cast<unsigned char>(seeded[headerSize + 5uLL]) ^ 0x01u));
				ASSERT_TRUE(io.good());
			}
			replaceA(opened.value(), ids, keep);
			const auto result = commit_package_changes(opened.value());
			ASSERT_FALSE(result.has_value());
			EXPECT_EQ(result.error().kind, Kind::eDamagedSource);
		}

		auto entries = std::vector<std::filesystem::path> {};
		for (const auto& entry : std::filesystem::directory_iterator { dir }) { entries.push_back(entry.path().filename()); }
		EXPECT_EQ(entries, (std::vector<std::filesystem::path> { "t.hgpk" }));
		EXPECT_EQ(std::filesystem::file_size(path), seededSize);
		std::filesystem::remove_all(dir);
	}

	TEST(Commit, NonMonotonicIndexStillCorrect) {
		// Content X at 8..16, Y at 0..8; the index lists X first, so the new layout reads the old file backwards.
		Bytes raw(headerSize + 16uLL + 2uLL * sizeof(package::Indexed) + footerSize);
		const auto indexSize = 2uLL * sizeof(package::Indexed);
		PackageHeader header {
			.magicBytes = PackageMagicBytes,
			.magicVersion = PackageMagicVersion[0],
			.endianness = PackageEndianness::eBigEndian,
			.version = 1uLL,
			.guid = {},
			.compression = PackageCompression::eNone,
			.__reserved__ = {},
			.packageSize = 16uLL + indexSize,
			.inflatedSize = 0uLL,
			.indexOffset = headerSize + 16uLL,
			.indexSize = indexSize,
			.metaOffset = headerSize + 16uLL + indexSize,
			.metaSize = 0uLL,
			.compDataOffset = headerSize + 16uLL + indexSize,
			.compDataSize = 0uLL
		};
		PackageFooter footer {
			.crc32 = {},
			.__reserved__ = {},
			.magicBytes = PackageMagicBytes,
			.magicVersion = PackageMagicVersion[0],
			.endianess = PackageEndianness::eBigEndian
		};
		std::memcpy(raw.data(), &header, headerSize);
		for (auto i = 0uLL; i < 16uLL; ++i) { raw[headerSize + i] = static_cast<_::byte>(i < 8uLL ? 'y' : 'x'); }
		const auto xGuid = ArchiveGuid::random();
		const auto yGuid = ArchiveGuid::random();
		auto slot = 0uLL;
		for (const auto& [offset, guid] : { std::pair { 8, xGuid }, std::pair { 0, yGuid } }) {
			package::Indexed indexed {};
			indexed.data.offset = offset;
			indexed.data.size = 8;
			indexed.header.type = ArchiveType::eRaw;
			indexed.header.guid = guid;
			std::memcpy(raw.data() + header.indexOffset + (slot++) * sizeof(package::Indexed), &indexed, sizeof(indexed));
		}
		std::memcpy(raw.data() + headerSize + header.packageSize, &footer, footerSize);
		auto crc = engine::crypto::Crc32 {};
		crc.update({ raw.data(), static_cast<size_t>(headerSize + header.packageSize) });
		const auto value = std::move(crc).finalize();
		const u64 stored = std::endian::native == std::endian::little ? std::byteswap(value) : value;
		std::memcpy(raw.data() + headerSize + header.packageSize, &stored, sizeof(stored));

		Bytes big(raw.size() + 64uLL);
		std::ranges::copy(raw, big.begin());
		auto bigBlob = ByteSpanBlob { std::span<_::byte> { big.data(), big.size() }, raw.size() };
		auto sink = engine::diag::CollectingSink {};
		auto opened = open_read_write_package(bigBlob, "t.hgpk", sink);
		ASSERT_TRUE(opened.has_value());

		Keep keep;
		for (auto& linked : opened.value().getLinker()) {
			if (linked.first == yGuid) {
				linked.second.changes.emplace_back(package::ArchiveDeltaReplace { .where = 0, .byteCount = 8, .withSizedData = keep.make("abc") });
			}
		}
		ASSERT_TRUE(commit_package_changes(opened.value()).has_value());
		EXPECT_EQ(sortedTexts(big, static_cast<size_t>(bigBlob.size())), (std::vector<std::string> { "abc", "xxxxxxxx" }));
	}

	TEST(Commit, CreatesEmptyPackage) {
		Bytes raw(1024uLL);
		auto blob = ByteSpanBlob { std::span<_::byte> { raw.data(), raw.size() }, 0uLL };
		auto package = make_read_write_package(blob);
		ASSERT_TRUE(commit_package_changes(package).has_value());
		EXPECT_EQ(contents(raw, static_cast<size_t>(blob.size())).size(), 0uLL);

		Keep keep;
		auto& linked = package.getLinker().add({ .type = ArchiveType::eUndefined, .guid = ArchiveGuid::random() });
		linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = 0, .sizedData = keep.make("AAAA") });
		ASSERT_TRUE(commit_package_changes(package).has_value());
		EXPECT_EQ(sortedTexts(raw, static_cast<size_t>(blob.size())), (std::vector<std::string> { "AAAA" }));
	}

	TEST(Commit, MemoryAndFileGiveSameBytes) {
		const auto path = std::filesystem::temp_directory_path() / "heliogrim_commit_same.hgpk";

		Ids ids;
		Keep keep;
		Bytes seeded(8192uLL);
		auto seedBlob = ByteSpanBlob { std::span<_::byte> { seeded.data(), seeded.size() }, 0uLL };
		seed(seedBlob, ids, keep);
		const auto seededSize = static_cast<size_t>(seedBlob.size());
		{
			std::ofstream out { path, std::ios::binary | std::ios::trunc };
			out.write(reinterpret_cast<const char*>(seeded.data()), static_cast<std::streamsize>(seededSize));
		}

		// Both sides open the same bytes, so their linkers list the archives in the same order.
		auto commitOpened = [&](ref<Blob> blob_) {
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(blob_, "t.hgpk", sink);
			ASSERT_TRUE(opened.has_value());
			replaceA(opened.value(), ids, keep);
			ASSERT_TRUE(commit_package_changes(opened.value()).has_value());
		};

		Bytes raw = seeded;
		auto memory = ByteSpanBlob { std::span<_::byte> { raw.data(), raw.size() }, seededSize };
		commitOpened(memory);
		{
			LfsSyncBlob file { fs::Path { path } };
			commitOpened(file);
		}

		// Note: The file blob still reads the unlinked old file; read the result back through std.
		std::ifstream in { path, std::ios::binary };
		const std::string text((std::istreambuf_iterator<char> { in }), std::istreambuf_iterator<char> {});
		in.close();
		Bytes fileBytes;
		for (const auto ch : text) { fileBytes.push_back(static_cast<_::byte>(ch)); }
		raw.resize(static_cast<size_t>(memory.size()));
		EXPECT_NE(raw.size(), seededSize);
		EXPECT_EQ(fileBytes, raw);
		std::filesystem::remove(path);
	}

	TEST(Commit, ShortWriteIntoRealTempKeepsFileAndLeavesNoTemp) {
		const auto dir = std::filesystem::temp_directory_path() / "heliogrim_commit_short";
		std::filesystem::remove_all(dir);
		std::filesystem::create_directories(dir);
		const auto path = dir / "t.hgpk";

		Ids ids;
		Keep keep;
		Bytes seeded(8192uLL);
		auto seedBlob = ByteSpanBlob { std::span<_::byte> { seeded.data(), seeded.size() }, 0uLL };
		seed(seedBlob, ids, keep);
		const auto seededSize = static_cast<size_t>(seedBlob.size());
		{
			std::ofstream out { path, std::ios::binary | std::ios::trunc };
			out.write(reinterpret_cast<const char*>(seeded.data()), static_cast<std::streamsize>(seededSize));
		}

		for (const auto failAt : { 0, 3 }) {
			FileFault file { Rc<LfsSyncBlob>::create(fs::Path { path }), failAt };
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(file, "t.hgpk", sink);
			ASSERT_TRUE(opened.has_value());
			replaceA(opened.value(), ids, keep);
			const auto result = commit_package_changes(opened.value());
			ASSERT_FALSE(result.has_value());
			EXPECT_EQ(result.error().kind, Kind::eDiskFull);
		}

		auto entries = std::vector<std::filesystem::path> {};
		for (const auto& entry : std::filesystem::directory_iterator { dir }) { entries.push_back(entry.path().filename()); }
		EXPECT_EQ(entries, (std::vector<std::filesystem::path> { "t.hgpk" }));
		EXPECT_EQ(std::filesystem::file_size(path), seededSize);
		std::filesystem::remove_all(dir);
	}

	TEST(Commit, MemoryAndIoFileGiveSameBytes) {
		const auto path = std::filesystem::temp_directory_path() / "heliogrim_commit_same_io.hgpk";

		Ids ids;
		Keep keep;
		Bytes seeded(8192uLL);
		auto seedBlob = ByteSpanBlob { std::span<_::byte> { seeded.data(), seeded.size() }, 0uLL };
		seed(seedBlob, ids, keep);
		const auto seededSize = static_cast<size_t>(seedBlob.size());
		{
			std::ofstream out { path, std::ios::binary | std::ios::trunc };
			out.write(reinterpret_cast<const char*>(seeded.data()), static_cast<std::streamsize>(seededSize));
		}

		auto commitOpened = [&](ref<Blob> blob_) {
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(blob_, "t.hgpk", sink);
			ASSERT_TRUE(opened.has_value());
			replaceA(opened.value(), ids, keep);
			ASSERT_TRUE(commit_package_changes(opened.value()).has_value());
		};

		Bytes raw = seeded;
		auto memory = ByteSpanBlob { std::span<_::byte> { raw.data(), raw.size() }, seededSize };
		commitOpened(memory);

		engine::io::IoSystem io {};
		io.start(); {
			auto file = await(io.open(path, engine::io::IoOpenMode::eReadWrite));
			ASSERT_TRUE(file.has_value());
			auto blob = Rc<IoFileBlob>::create(io, std::move(file).value(), io.createQueue(engine::io::IoPriority::eNormal), path);
			commitOpened(*blob);
		}
		io.stop();

		std::ifstream in { path, std::ios::binary };
		const std::string text((std::istreambuf_iterator<char> { in }), std::istreambuf_iterator<char> {});
		in.close();
		Bytes fileBytes;
		for (const auto ch : text) { fileBytes.push_back(static_cast<_::byte>(ch)); }
		raw.resize(static_cast<size_t>(memory.size()));
		EXPECT_EQ(fileBytes, raw);
		std::filesystem::remove(path);
	}

	TEST(Commit, CopyWindowsSplitLargeRanges) {
		constexpr auto gib = 1uLL << 30uLL;
		const auto windows = copyWindows(3uLL * gib, 5uLL * gib);
		auto next = 3uLL * gib;
		auto sum = 0uLL;
		for (const auto& [offset, size] : windows) {
			EXPECT_EQ(offset, next);
			EXPECT_LE(size, 1uLL << 20uLL);
			next += size;
			sum += size;
		}
		EXPECT_EQ(sum, 5uLL * gib);
	}

	// W03.9 (#234) cost check: replace commits of one 64 KiB archive in a 100 MiB package on an LfsSyncBlob,
	// at least `minRounds_` rounds and until `duration_` passed. The commit flushes inside finishReplace.
	// Set HG_COMMIT_BENCH_DIR to a directory on the drive under test.
	void benchReplaceCommits(const int minRounds_, const std::chrono::seconds duration_) {

		const auto* const dir = std::getenv("HG_COMMIT_BENCH_DIR");
		ASSERT_NE(dir, nullptr);

		constexpr auto archiveSize = 64uz * 1024uz;
		constexpr auto archiveCount = 1600uz;

		std::filesystem::create_directories(dir);
		const auto file = std::filesystem::path { dir } / "engine-commit-bench.hgpkg";
		std::filesystem::remove(file);

		auto archives = Vector<uptr<BufferArchive>> {};
		auto target = ArchiveGuid {};
		{
			auto blob = LfsSyncBlob { fs::Path { file } };
			auto package = make_read_write_package(blob);
			// Note: As BlobPack does for an empty blob.
			ASSERT_TRUE(writeHeader(package.getHeader(), blob));
			ASSERT_TRUE(writeLinker(package.getHeader(), package.getLinker(), blob));
			ASSERT_TRUE(writeFooter(package.getHeader(), package.getFooter(), blob));

			for (auto index = 0uz; index < archiveCount; ++index) {
				auto archive = make_uptr<BufferArchive>();
				archive->resize(archiveSize);
				archive->setType(ArchiveType::eRaw);
				const auto guid = ArchiveGuid::random();
				if (index == archiveCount / 2) {
					target = guid;
				}
				auto& linked = package.getLinker().add({ .type = ArchiveType::eUndefined, .guid = guid });
				linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = streamoff {}, .sizedData = *archive });
				archives.emplace_back(::hg::move(archive));
			}
			ASSERT_TRUE(commit_package_changes(package).has_value());
		}

		// Note: A committed file blob is spent, so every round opens the package anew (not timed).
		auto rounds = 0;
		auto totalMs = 0.0;
		auto minMs = std::numeric_limits<double>::max();
		auto maxMs = 0.0;
		const auto until = std::chrono::steady_clock::now() + duration_;
		for (; rounds < minRounds_ || std::chrono::steady_clock::now() < until; ++rounds) {
			auto blob = LfsSyncBlob { fs::Path { file } };
			auto sink = engine::diag::CollectingSink {};
			auto opened = open_read_write_package(blob, "bench.hgpkg", sink);
			ASSERT_TRUE(opened.has_value());
			auto& package = opened.value();

			auto replacement = make_uptr<BufferArchive>();
			replacement->resize(archiveSize);
			replacement->setType(ArchiveType::eRaw);
			auto linked = package.getLinker()[target];
			ASSERT_NE(linked, None);
			linked->changes.emplace_back(
				package::ArchiveDeltaReplace { .where = streamoff {}, .byteCount = static_cast<streamsize>(archiveSize), .withSizedData = *replacement }
			);

			auto stats = CommitStats {};
			const auto start = std::chrono::steady_clock::now();
			const auto result = commit_package_changes(package, &stats);
			const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			ASSERT_TRUE(result.has_value()) << "kind " << static_cast<int>(result.error().kind) << " os " << result.error().osError;
			std::printf(
				"replace commit (flushed), one 64 KiB archive: %.2f ms, old %llu B, new %llu B, written %llu B\n",
				elapsed,
				static_cast<unsigned long long>(stats.oldSize),
				static_cast<unsigned long long>(stats.newSize),
				static_cast<unsigned long long>(stats.written)
			);
			totalMs += elapsed;
			minMs = (std::min)(minMs, elapsed);
			maxMs = (std::max)(maxMs, elapsed);
		}
		std::printf("%d rounds: mean %.2f ms, min %.2f ms, max %.2f ms\n", rounds, totalMs / rounds, minMs, maxMs);
		std::filesystem::remove(file);
	}

	TEST(PackageBench, DISABLED_CommitReplaceOneArchive) {
		benchReplaceCommits(5, std::chrono::seconds { 0 });
	}

	// For profiler sessions (uProf, ETW): loops replace commits for HG_COMMIT_BENCH_SECONDS (default 30).
	TEST(PackageBench, DISABLED_CommitReplaceLoop) {
		const auto* const seconds = std::getenv("HG_COMMIT_BENCH_SECONDS");
		benchReplaceCommits(1, std::chrono::seconds { seconds != nullptr ? std::atoi(seconds) : 30 });
	}

	TEST(Commit, ReplacedFlushFailureSaysDurabilityUnconfirmed) {
		const engine::io::CommitError replaced { .kind = Kind::eFlushFailed, .path = "p.hgpk", .replaced = true };
		const engine::io::CommitError kept { .kind = Kind::eFlushFailed, .path = "p.hgpk" };

		EXPECT_NE(describe(replaced).find("durability is unconfirmed"), String::npos);
		EXPECT_EQ(describe(kept).find("durability is unconfirmed"), String::npos);
		EXPECT_NE(engine::diag::render(toDiagnostic(replaced, "p.hgpk")).find("durability is unconfirmed"), String::npos);
	}
}
