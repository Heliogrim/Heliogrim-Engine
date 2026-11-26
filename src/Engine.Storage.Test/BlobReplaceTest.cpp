#include "pch.h"

/**/

#include "Scratch.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.Resource.Blob/File/ReplaceSequence.hpp>

import Engine.Async;
import Engine.IO.Dispatcher;
import Engine.Resource.Blob;
import std;

using namespace hg;
using namespace hg::engine::resource;
using namespace hg::engine::io;
using namespace hg::engine;

namespace StorageModule {
	namespace {
		[[nodiscard]] Vector<_::byte> pattern(const size_t size_, const u8 seed_) {
			Vector<_::byte> out(size_);
			for (size_t i = 0; i < size_; ++i) {
				out[i] = static_cast<_::byte>(seed_ + i);
			}
			return out;
		}
	}

	TEST(BlobReplace, SpanReplaceCopiesImage) {
		auto storage = pattern(64uz, 1u);
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		auto image = pattern(40uz, 100u);
		auto begun = blob.beginReplace(40);
		ASSERT_TRUE(begun.has_value());
		auto replacement = std::move(begun).value();
		replacement->write(0, std::span<_::byte> { image });

		ASSERT_TRUE(blob.finishReplace(std::move(replacement)).has_value());
		EXPECT_EQ(blob.size(), 40);
		EXPECT_TRUE(std::equal(image.begin(), image.end(), storage.begin()));
	}

	TEST(BlobReplace, SpanReplaceTooLargeIsDiskFull) {
		auto storage = pattern(64uz, 1u);
		const auto before = storage;
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		auto image = pattern(80uz, 100u);
		auto begun = blob.beginReplace(80);
		ASSERT_TRUE(begun.has_value());
		auto replacement = std::move(begun).value();
		replacement->write(0, std::span<_::byte> { image });

		auto done = blob.finishReplace(std::move(replacement));
		ASSERT_FALSE(done.has_value());
		EXPECT_EQ(done.error().kind, CommitError::Kind::eDiskFull);
		EXPECT_EQ(done.error().bytesNeeded, 80uLL);
		EXPECT_EQ(blob.size(), 16);
		EXPECT_EQ(storage, before);
	}

	TEST(BlobReplace, DroppedReplacementLeavesSpan) {
		auto storage = pattern(64uz, 1u);
		const auto before = storage;
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		{
			auto image = pattern(40uz, 100u);
			auto begun = blob.beginReplace(40);
			ASSERT_TRUE(begun.has_value());
			auto replacement = std::move(begun).value();
			replacement->write(0, std::span<_::byte> { image });
		}

		EXPECT_EQ(blob.size(), 16);
		EXPECT_EQ(storage, before);
	}

	TEST(BlobReplace, WindowedBlobUnsupported) {
		auto storage = pattern(64uz, 1u);
		ByteSpanBlob source { std::span<_::byte> { storage } };
		WindowedBlob windowed { source, 0, 64, 16uz };

		auto begun = windowed.beginReplace(8);
		ASSERT_FALSE(begun.has_value());
		EXPECT_EQ(begun.error().kind, CommitError::Kind::eUnsupportedVolume);
	}

	/**/

	namespace {
		[[nodiscard]] bool hasTemp(ref<const std::filesystem::path> dir_) {
			for (const auto& entry : std::filesystem::directory_iterator { dir_ }) {
				if (entry.path().extension() == ::hg::engine::io::temp_suffix) {
					return true;
				}
			}
			return false;
		}

		void put(ref<const std::filesystem::path> path_, const std::string_view text_) {
			std::ofstream out { path_, std::ios::binary };
			out.write(text_.data(), static_cast<std::streamsize>(text_.size()));
		}

		[[nodiscard]] std::string slurp(ref<Blob> blob_) {
			std::array<_::byte, 64> buf {};
			const auto got = blob_.read(0, std::span<_::byte> { buf });
			std::string out {};
			for (const auto b : got) {
				out.push_back(static_cast<char>(b));
			}
			return out;
		}

		[[nodiscard]] Vector<_::byte> bytesOf(const std::string_view text_) {
			Vector<_::byte> out {};
			for (const auto c : text_) {
				out.push_back(static_cast<_::byte>(c));
			}
			return out;
		}

		template <typename Blob_>
		struct BlobFactory;

		template <>
		struct BlobFactory<LfsSyncBlob> {
			io::IoSystem* _io = nullptr;

			[[nodiscard]] Rc<Blob> open(ref<const std::filesystem::path> path_) const {
				return Rc<LfsSyncBlob>::create(hg::fs::Path { path_ });
			}
		};

		template <>
		struct BlobFactory<IoFileBlob> {
			io::IoSystem* _io = nullptr;

			[[nodiscard]] Rc<Blob> open(ref<const std::filesystem::path> path_) const {
				auto opened = await(_io->open(path_, io::IoOpenMode::eReadWrite));
				EXPECT_TRUE(opened.has_value());
				return Rc<IoFileBlob>::create(*_io, std::move(opened).value(), _io->createQueue(io::IoPriority::eNormal), path_);
			}
		};
	}

	template <typename Blob_>
	class FileReplace :
		public testing::Test {
	public:
		FileReplace() {
			_io.start();
			_factory._io = &_io;
			_target = _scratch.path / "package.bin";
		}

		~FileReplace() override {
			_io.stop();
		}

	public:
		ScratchDir _scratch {};
		io::IoSystem _io {};
		BlobFactory<Blob_> _factory {};
		std::filesystem::path _target;

	public:
		// Returns the commit result; blobs are gone before the caller reads the file again.
		[[nodiscard]] Result<void, CommitError> replaceWith(ref<Blob> blob_, const std::string_view text_) {
			auto begun = blob_.beginReplace(static_cast<streamsize>(text_.size()));
			if (not begun.has_value()) {
				return Result<void, CommitError>::unexpected(begun.error());
			}
			auto replacement = std::move(begun).value();
			auto image = bytesOf(text_);
			replacement->write(0, std::span<_::byte> { image });
			return blob_.finishReplace(std::move(replacement));
		}
	};

	using FileBlobTypes = testing::Types<LfsSyncBlob, IoFileBlob>;
	TYPED_TEST_SUITE(FileReplace, FileBlobTypes);

	TYPED_TEST(FileReplace, SwapsContent) {
		put(this->_target, "old"); {
			auto blob = this->_factory.open(this->_target);
			ASSERT_TRUE(this->replaceWith(*blob, "newer").has_value());
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "newer");
	}

	TYPED_TEST(FileReplace, SpentAfterReplace) {
		put(this->_target, "old"); {
			auto blob = this->_factory.open(this->_target);
			ASSERT_TRUE(this->replaceWith(*blob, "first").has_value());

			alignas(4096) std::array<_::byte, 4096> buf {};
			EXPECT_EQ(blob->read(0, std::span<_::byte> { buf }).size(), 0uz);
			EXPECT_EQ(blob->lastError(), io::error_invalid_handle);
			EXPECT_EQ(blob->write(0, std::span<_::byte> { buf }).size(), 0uz);
			EXPECT_FALSE(blob->trim(1uz));
			EXPECT_FALSE(blob->beginReplace(5).has_value());
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "first");
	}

	TYPED_TEST(FileReplace, DroppedLeavesTargetAndNoTemp) {
		put(this->_target, "old"); {
			auto blob = this->_factory.open(this->_target);
			auto begun = blob->beginReplace(5);
			ASSERT_TRUE(begun.has_value());
			EXPECT_TRUE(hasTemp(this->_scratch.path));
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "old");
	}

	TYPED_TEST(FileReplace, LockedAfterRetries) {
		put(this->_target, "old"); {
			auto blob = this->_factory.open(this->_target);
			const auto lock = ::CreateFileW(this->_target.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
			ASSERT_NE(lock, INVALID_HANDLE_VALUE);

			auto done = this->replaceWith(*blob, "newer");
			::CloseHandle(lock);

			ASSERT_FALSE(done.has_value());
			EXPECT_TRUE(done.error().kind == CommitError::Kind::eLocked || done.error().kind == CommitError::Kind::eAccessDenied);
			EXPECT_TRUE(done.error().osError == 32u || done.error().osError == 5u);
			EXPECT_GE(done.error().retriedMs, 1500u);
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "old");
	}

	TYPED_TEST(FileReplace, SpaceCheckFailsBeforeTemp) {
		put(this->_target, "old");
		auto blob = this->_factory.open(this->_target);
		auto begun = blob->beginReplace((std::numeric_limits<streamsize>::max)() / 2);
		ASSERT_FALSE(begun.has_value());
		EXPECT_EQ(begun.error().kind, CommitError::Kind::eDiskFull);
		EXPECT_GT(begun.error().bytesNeeded, 0uLL);
		EXPECT_FALSE(hasTemp(this->_scratch.path));
	}

	TYPED_TEST(FileReplace, SweepsStaleTemp) {
		put(this->_target, "old");
		const auto stale = this->_scratch.path / "package.bin.deadbeef.hgtmp";
		put(stale, "stale");

		auto blob = this->_factory.open(this->_target);
		auto begun = blob->beginReplace(5);
		ASSERT_TRUE(begun.has_value());
		EXPECT_FALSE(std::filesystem::exists(stale));
	}

	TYPED_TEST(FileReplace, TargetDeletedMidway) {
		put(this->_target, "old"); {
			auto blob = this->_factory.open(this->_target);
			auto begun = blob->beginReplace(5);
			ASSERT_TRUE(begun.has_value());
			auto replacement = std::move(begun).value();
			auto image = bytesOf("newer");
			replacement->write(0, std::span<_::byte> { image });

			std::error_code ec {};
			ASSERT_TRUE(std::filesystem::remove(this->_target, ec));

			// Either the target returns with the new content or the error is a value.
			const auto done = blob->finishReplace(std::move(replacement));
			if (done.has_value()) {
				EXPECT_TRUE(std::filesystem::exists(this->_target));
			} else {
				EXPECT_TRUE(done.error().kind == CommitError::Kind::eLocked || done.error().kind == CommitError::Kind::eAccessDenied);
				EXPECT_FALSE(std::filesystem::exists(this->_target));
			}
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
	}

	namespace {
		struct ForceByName {
			ForceByName() { forceByNameReplace = true; }
			~ForceByName() { forceByNameReplace = false; }
		};
	}

	TYPED_TEST(FileReplace, ByNameFallbackSwapsContent) {
		put(this->_target, "old"); {
			const ForceByName force {};
			auto blob = this->_factory.open(this->_target);
			ASSERT_TRUE(this->replaceWith(*blob, "newer").has_value());
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "newer");
	}

	TYPED_TEST(FileReplace, ByNameFallbackBlockedByQueryHandle) {
		put(this->_target, "old"); {
			const ForceByName force {};
			auto blob = this->_factory.open(this->_target);
			// A cached query handle without share-delete keeps the old name busy.
			const auto query = ::CreateFileW(this->_target.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
			ASSERT_NE(query, INVALID_HANDLE_VALUE);

			auto done = this->replaceWith(*blob, "newer");
			::CloseHandle(query);

			ASSERT_FALSE(done.has_value());
			EXPECT_EQ(done.error().kind, CommitError::Kind::eUnsupportedVolume);
			EXPECT_TRUE(done.error().osError == 32u || done.error().osError == 5u);
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "old");
	}

	TYPED_TEST(FileReplace, FinishRejectsForeignAndRepeatedReplacement) {
		put(this->_target, "old");
		const auto other = this->_scratch.path / "other.bin";
		put(other, "other"); {
			auto blob = this->_factory.open(this->_target);
			auto stranger = this->_factory.open(other);

			EXPECT_FALSE(blob->finishReplace(Rc<Blob> {}).has_value());
			EXPECT_FALSE(blob->finishReplace(Rc<Blob> { stranger }).has_value());
			auto bytes = bytesOf("mem");
			EXPECT_FALSE(blob->finishReplace(Rc<Blob> { Rc<ByteSpanBlob>::create(std::span<_::byte> { bytes }, bytes.size()) }).has_value());
			EXPECT_EQ(slurp(*blob), "old");

			auto begun = blob->beginReplace(5);
			ASSERT_TRUE(begun.has_value());
			Rc<Blob> replacement = std::move(begun).value();
			auto image = bytesOf("newer");
			replacement->write(0, std::span<_::byte> { image });
			ASSERT_TRUE(blob->finishReplace(Rc<Blob> { replacement }).has_value());
			EXPECT_FALSE(blob->finishReplace(Rc<Blob> { replacement }).has_value());
		}
		EXPECT_FALSE(hasTemp(this->_scratch.path));
		auto reopened = this->_factory.open(this->_target);
		EXPECT_EQ(slurp(*reopened), "newer");
	}

	TEST(IoFileBlobFault, FailedCompletionIsReported) {
		const ScratchDir scratch {};
		const auto target = scratch.path / "package.bin";
		put(target, "old");

		io::IoSystem io {};
		io.start(); {
			auto opened = await(io.open(target, io::IoOpenMode::eRead));
			ASSERT_TRUE(opened.has_value());
			auto blob = Rc<IoFileBlob>::create(io, std::move(opened).value(), io.createQueue(io::IoPriority::eNormal), target);

			alignas(4096) std::array<_::byte, 4096> buf {};
			EXPECT_EQ(blob->write(0, std::span<_::byte> { buf }).size(), 0uz);
			EXPECT_NE(blob->lastError(), 0u);
		}
		io.stop();
		EXPECT_EQ(std::filesystem::file_size(target), 3uz);
	}

	TEST(LfsSyncBlobShare, SharesDelete) {
		const ScratchDir scratch {};
		const auto target = scratch.path / "package.bin";
		put(target, "old");

		LfsSyncBlob blob { hg::fs::Path { target } };
		auto temp = createTempSibling(target, 0u);
		ASSERT_TRUE(temp.has_value());
		const auto error = renameOver(temp->handle, target);
		::CloseHandle(temp->handle);
		EXPECT_EQ(error, 0u);
	}

	/**/

	namespace {
		struct Fake {
			u32 flushTemp = 0u;
			u32 renameTemp = 0u;
			u32 replaceByName = 0u;
			u32 flushTarget = 0u;

			u32 flushTempCalls = 0u;
			u32 renameCalls = 0u;
			u32 replaceCalls = 0u;
			u32 flushTargetCalls = 0u;
			bool lastByName = false;

			[[nodiscard]] ReplaceSteps steps() {
				return ReplaceSteps {
					.flushTemp = [this]() {
						++flushTempCalls;
						return flushTemp;
					},
					.renameTemp = [this]() {
						++renameCalls;
						return renameTemp;
					},
					.replaceByName = [this]() {
						++replaceCalls;
						return replaceByName;
					},
					.flushTarget = [this](const bool byName_) {
						++flushTargetCalls;
						lastByName = byName_;
						return flushTarget;
					}
				};
			}
		};

		// A target path that is not a file, so the read-only probe stays false.
		const std::filesystem::path fakeTarget { "no-such-dir/package.bin" };
	}

	TEST(ReplaceSteps, TempFlushFailureStopsBeforeRename) {
		Fake fake { .flushTemp = 1117u };
		bool swapped = false;
		const auto res = runReplaceSteps(fakeTarget, fake.steps(), swapped);
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eFlushFailed);
		EXPECT_EQ(fake.flushTempCalls, 1u);
		EXPECT_EQ(fake.renameCalls, 0u);
		EXPECT_FALSE(swapped);
	}

	TEST(ReplaceSteps, NonRetryableRenameErrorDoesNotSleep) {
		Fake fake { .renameTemp = 1u };
		bool swapped = false;
		const auto res = runReplaceSteps(fakeTarget, fake.steps(), swapped);
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().retriedMs, 0u);
		EXPECT_EQ(res.error().osError, 1u);
		EXPECT_EQ(fake.renameCalls, 1u);
		EXPECT_EQ(fake.replaceCalls, 0u);
		EXPECT_FALSE(swapped);
	}

	TEST(ReplaceSteps, UnsupportedRenameFallsBackOnce) {
		Fake fake { .renameTemp = 87u, .replaceByName = 5u };
		bool swapped = false;
		const auto res = runReplaceSteps(fakeTarget, fake.steps(), swapped);
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eUnsupportedVolume);
		EXPECT_EQ(res.error().osError, 5u);
		EXPECT_EQ(fake.replaceCalls, 1u);
		EXPECT_EQ(fake.flushTargetCalls, 0u);
		EXPECT_FALSE(swapped);
	}

	TEST(ReplaceSteps, FallbackSuccessFlushesByName) {
		Fake fake { .renameTemp = 87u };
		bool swapped = false;
		ASSERT_TRUE(runReplaceSteps(fakeTarget, fake.steps(), swapped).has_value());
		EXPECT_EQ(fake.replaceCalls, 1u);
		EXPECT_EQ(fake.flushTargetCalls, 1u);
		EXPECT_TRUE(fake.lastByName);
		EXPECT_TRUE(swapped);
	}

	TEST(ReplaceSteps, TargetFlushFailure) {
		Fake fake { .flushTarget = 1117u };
		bool swapped = false;
		const auto res = runReplaceSteps(fakeTarget, fake.steps(), swapped);
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eFlushFailed);
		EXPECT_TRUE(res.error().replaced);
		EXPECT_EQ(fake.flushTargetCalls, 1u);
		EXPECT_TRUE(swapped);
	}

	TEST(ReplaceSteps, RetryThenSuccess) {
		Fake fake {};
		auto steps = fake.steps();
		u32 renames = 0u;
		steps.renameTemp = [&renames]() { return ++renames == 1u ? 32u : 0u; };
		bool swapped = false;
		ASSERT_TRUE(runReplaceSteps(fakeTarget, steps, swapped).has_value());
		EXPECT_EQ(renames, 2u);
		EXPECT_EQ(fake.flushTargetCalls, 1u);
		EXPECT_FALSE(fake.lastByName);
		EXPECT_TRUE(swapped);
	}

	TEST(ReplaceSteps, SpaceCheckInMissingDirectoryReportsOsError) {
		const ScratchDir scratch {};
		const auto full = checkReplaceSpace(scratch.path / "missing" / "package.bin", 5);
		ASSERT_TRUE(full.has_value());
		EXPECT_NE(full->osError, 0u);
		EXPECT_NE(full->kind, CommitError::Kind::eDiskFull);
		EXPECT_EQ(full->bytesNeeded, 0uLL);
	}

	TEST(FileOps, CreateTempSiblingInMissingDirectoryFails) {
		const ScratchDir scratch {};
		const auto temp = createTempSibling(scratch.path / "missing" / "package.bin", 0u);
		ASSERT_FALSE(temp.has_value());
		EXPECT_NE(temp.error().osError, 0u);
	}

	TEST(HeapBlobBounds, WriteAndReadGuards) {
		auto storage = pattern(8uz, 1u);
		ByteSpanBlob blob { std::span<_::byte> { storage }, 0uz };
		auto begun = blob.beginReplace(0);
		ASSERT_TRUE(begun.has_value());
		auto heap = std::move(begun).value();

		auto image = pattern(4uz, 9u);
		EXPECT_EQ(heap->write(-1, std::span<_::byte> { image }).size(), 0uz);
		EXPECT_EQ(heap->write(1, std::span<_::byte> { image }).size(), 0uz);
		EXPECT_EQ(heap->size(), 0);
		EXPECT_EQ(heap->write(0, std::span<_::byte> { image }).size(), 4uz);
		EXPECT_EQ(heap->write(4, std::span<_::byte> { image }).size(), 4uz);
		EXPECT_EQ(heap->write(9, std::span<_::byte> { image }).size(), 0uz);
		EXPECT_EQ(heap->size(), 8);

		std::array<_::byte, 16> out {};
		EXPECT_EQ(heap->read(-1, std::span<_::byte> { out }).size(), 0uz);
		EXPECT_EQ(heap->read(9, std::span<_::byte> { out }).size(), 0uz);
		EXPECT_EQ(heap->read(8, std::span<_::byte> { out }).size(), 0uz);
		EXPECT_EQ(heap->read(6, std::span<_::byte> { out }).size(), 2uz);
		EXPECT_EQ(heap->read(0, std::span<_::byte> { out }).size(), 8uz);

		EXPECT_TRUE(heap->trim(100uz));
		EXPECT_EQ(heap->size(), 0);
	}

	TEST(ByteSpanBlobReplace, NullReplacementFails) {
		std::array<_::byte, 8> raw {};
		ByteSpanBlob blob { std::span<_::byte> { raw }, 0uz };
		EXPECT_FALSE(blob.finishReplace(Rc<Blob> {}).has_value());
	}

	TEST(ByteSpanBlobReplace, ShortReadLeavesSpanUntouched) {
		// Claims more bytes than its read hands out.
		class ShortBlob final :
			public InheritMeta<ShortBlob, Blob> {
		public:
			std::span<_::byte> read(streamoff, mref<std::span<_::byte>> dst_) const override {
				for (auto& b : dst_) { b = static_cast<_::byte>('x'); }
				return std::move(dst_).subspan(0uz, dst_.size() - 1uz);
			}
			std::span<_::byte> write(streamoff, mref<std::span<_::byte>> src_) override { return std::move(src_); }
			bool trim(size_t) override { return false; }
			streamsize size() const noexcept override { return 4LL; }
			u32 lastError() const noexcept override { return 0u; }
		};

		auto raw = bytesOf("oldold");
		ByteSpanBlob blob { std::span<_::byte> { raw }, raw.size() };
		const auto res = blob.finishReplace(Rc<Blob> { Rc<ShortBlob>::create() });
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eDamagedSource);
		EXPECT_EQ(slurp(blob), "oldold");
	}

	#if ENV_WIN
	TEST(ReplaceSteps, AccessDeniedOnReadOnlyTargetFailsFast) {
		const ScratchDir scratch {};
		const auto target = scratch.path / "package.bin";
		put(target, "old");
		ASSERT_NE(::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_READONLY), 0);

		Fake fake { .renameTemp = 5u };
		bool swapped = false;
		const auto res = runReplaceSteps(target, fake.steps(), swapped);
		::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_NORMAL);

		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eReadOnly);
		EXPECT_EQ(res.error().retriedMs, 0u);
		EXPECT_EQ(fake.renameCalls, 1u);
	}

	TEST(ReplaceSteps, TargetTurnsReadOnlyWhileRetrying) {
		const ScratchDir scratch {};
		const auto target = scratch.path / "package.bin";
		put(target, "old");
		ASSERT_NE(::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_READONLY), 0);

		Fake fake {};
		auto steps = fake.steps();
		u32 renames = 0u;
		steps.renameTemp = [&renames]() { return ++renames == 1u ? 32u : 5u; };
		bool swapped = false;
		const auto res = runReplaceSteps(target, steps, swapped);
		::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_NORMAL);

		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().kind, CommitError::Kind::eReadOnly);
		EXPECT_EQ(res.error().retriedMs, 100u);
		EXPECT_EQ(renames, 2u);
		EXPECT_FALSE(swapped);
	}

	TEST(LfsSyncBlobReplace, ReadOnlyBlobRefusesBeginReplace) {
		const ScratchDir scratch {};
		const auto target = scratch.path / "package.bin";
		put(target, "old");
		ASSERT_NE(::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_READONLY), 0);

		{
			LfsSyncBlob blob { hg::fs::Path { target } };
			auto begun = blob.beginReplace(5);
			ASSERT_FALSE(begun.has_value());
			EXPECT_EQ(begun.error().kind, CommitError::Kind::eReadOnly);
			EXPECT_FALSE(hasTemp(scratch.path));
		}
		::SetFileAttributesW(target.c_str(), FILE_ATTRIBUTE_NORMAL);
	}
	#endif
}
