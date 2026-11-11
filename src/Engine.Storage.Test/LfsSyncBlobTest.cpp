#include "pch.h"

/**/

#include "Scratch.hpp"

/**/

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>

/**/

using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	TEST(LfsSyncBlobTest, ConcurrentReadsReturnOwnRanges) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";

		std::array<_::byte, 4096> pattern {};
		for (size_t i = 0; i < pattern.size(); ++i) {
			pattern[i] = static_cast<_::byte>(i % 251);
		} {
			std::ofstream out { path, std::ios::binary };
			out.write(reinterpret_cast<const char*>(pattern.data()), pattern.size());
		}

		std::atomic_size_t mismatches { 0 }; {
			LfsSyncBlob blob { fs::Path { path } };

			std::vector<std::thread> threads {};
			for (size_t k = 0; k < 8; ++k) {
				threads.emplace_back(
					[&blob, &pattern, &mismatches, k]() {
						std::array<_::byte, 512> buf {};
						for (size_t n = 0; n < 1000; ++n) {
							buf.fill(static_cast<_::byte>(0));
							const auto got = blob.read(static_cast<streamoff>(k * 512), std::span<_::byte> { buf });
							if (got.size() != buf.size() || not std::equal(buf.begin(), buf.end(), pattern.begin() + k * 512)) {
								++mismatches;
							}
						}
					}
				);
			}
			for (auto& t : threads) {
				t.join();
			}
		}

		EXPECT_EQ(mismatches.load(), 0uz);
	}

	TEST(LfsSyncBlobTest, ReadOnlyFileReads) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin"; {
			std::ofstream { path, std::ios::binary } << "read only";
		}
		std::filesystem::permissions(
			path,
			std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write,
			std::filesystem::perm_options::remove
		);

		std::array<_::byte, 32> buf {};
		size_t size = 0; {
			const LfsSyncBlob blob { fs::Path { path } };
			size = blob.read(0, std::span<_::byte> { buf }).size();
		}

		std::filesystem::permissions(path, std::filesystem::perms::owner_write, std::filesystem::perm_options::add);

		EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(buf.data()), size), "read only");
	}

	namespace {
		void writeFile(const std::filesystem::path& path_, std::string_view text_) {
			std::ofstream { path_, std::ios::binary | std::ios::trunc } << text_;
		}

		void makeReadOnly(const std::filesystem::path& path_) {
			std::filesystem::permissions(
				path_,
				std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write,
				std::filesystem::perm_options::remove
			);
		}
	}

	TEST(LfsSyncBlobDeathTest, WriteOnReadOnlyBlobAsserts) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "read only");
		makeReadOnly(path);

		LfsSyncBlob blob { fs::Path { path } };
		std::array<_::byte, 4> src { _::byte { 1 }, _::byte { 2 }, _::byte { 3 }, _::byte { 4 } };
		EXPECT_DEATH({ blob.write(0, std::span<_::byte> { src }); }, "");
		// Note: The parent process is untouched by the death test; the content stays intact.
		std::array<_::byte, 16> buf {};
		EXPECT_EQ(blob.read(0, std::span<_::byte> { buf }).size(), 9uz);
	}

	TEST(LfsSyncBlobDeathTest, TrimOnReadOnlyBlobAsserts) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "read only");
		makeReadOnly(path);

		LfsSyncBlob blob { fs::Path { path } };
		EXPECT_DEATH({ blob.trim(3uz); }, "");
		EXPECT_EQ(blob.size(), 9);
	}

	TEST(LfsSyncBlobTest, ReadAtEndYieldsEmptySpan) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "abcdef");

		LfsSyncBlob blob { fs::Path { path } };
		std::array<_::byte, 8> buf {};
		const auto got = blob.read(6, std::span<_::byte> { buf });
		EXPECT_TRUE(got.empty());
		// Note: The blob stays usable after an empty read at the end.
		EXPECT_EQ(blob.read(0, std::span<_::byte> { buf }).size(), 6uz);
	}

	TEST(LfsSyncBlobDeathTest, ReadBeyondEndAsserts) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "abcdef");

		LfsSyncBlob blob { fs::Path { path } };
		std::array<_::byte, 8> buf {};
		EXPECT_DEATH({ blob.read(7, std::span<_::byte> { buf }); }, "");
		EXPECT_DEATH({ blob.read(-1, std::span<_::byte> { buf }); }, "");
	}

	TEST(LfsSyncBlobTest, ReadStraddlingEndYieldsPartialSize) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "abcdef");

		LfsSyncBlob blob { fs::Path { path } };
		std::array<_::byte, 8> buf {};
		const auto got = blob.read(4, std::span<_::byte> { buf });
		ASSERT_EQ(got.size(), 2uz);
		EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(got.data()), got.size()), "ef");
	}

	TEST(LfsSyncBlobTest, ReadAfterShortReadSucceeds) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "abcdef");

		LfsSyncBlob blob { fs::Path { path } };
		std::array<_::byte, 8> buf {};
		ASSERT_EQ(blob.read(4, std::span<_::byte> { buf }).size(), 2uz);

		// Note: A short read must not leave stream state behind that makes later calls on the shared blob abort.
		const auto got = blob.read(0, std::span<_::byte> { buf });
		ASSERT_EQ(got.size(), 6uz);
		EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(got.data()), got.size()), "abcdef");
		EXPECT_EQ(blob.size(), 6);
	}

	TEST(LfsSyncBlobTest, SizeOfEmptyFileIsZero) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";
		writeFile(path, "");

		const LfsSyncBlob blob { fs::Path { path } };
		EXPECT_EQ(blob.size(), 0);

		std::array<_::byte, 4> buf {};
		EXPECT_TRUE(blob.read(0, std::span<_::byte> { buf }).empty());
	}

	TEST(LfsSyncBlobTest, MissingParentDirectoriesAreCreated) {

		const ScratchDir scratch {};
		const auto root = scratch.path / "root";
		const auto path = root / "a" / "b" / "blob.bin";
		ASSERT_FALSE(std::filesystem::exists(root));

		LfsSyncBlob blob { fs::Path { path } };
		EXPECT_TRUE(std::filesystem::is_regular_file(path));
		EXPECT_EQ(blob.size(), 0);

		std::array<_::byte, 3> src { _::byte { 'x' }, _::byte { 'y' }, _::byte { 'z' } };
		EXPECT_EQ(blob.write(0, std::span<_::byte> { src }).size(), 3uz);
		EXPECT_EQ(blob.size(), 3);
	}

	TEST(LfsSyncBlobTest, ConcurrentDisjointWritesAndReadsKeepFinalContent) {

		const ScratchDir scratch {};
		const auto path = scratch.path / "blob.bin";

		constexpr size_t slots = 8uz;
		constexpr size_t slotSize = 256uz;
		// Note: Pre-size the file, writes must not leave holes.
		writeFile(path, std::string(slots * slotSize, '\0'));

		std::atomic_size_t bad {};
		{
			LfsSyncBlob blob { fs::Path { path } };

			// Note: Fails on a regression that interleaves seek and write of two threads (data lands at the wrong offset).
			std::vector<std::thread> threads {};
			for (size_t k = 0; k < slots; ++k) {
				threads.emplace_back(
					[&blob, &bad, k] {
						std::array<_::byte, slotSize> own {};
						std::array<_::byte, slotSize> got {};
						for (size_t n = 0; n < 200; ++n) {
							own.fill(static_cast<_::byte>(k + 1));
							if (blob.write(static_cast<streamoff>(k * slotSize), std::span<_::byte> { own }).size() != slotSize) {
								++bad;
							}
							const auto read = blob.read(static_cast<streamoff>(k * slotSize), std::span<_::byte> { got });
							if (read.size() != slotSize || not std::ranges::all_of(read, [k](_::byte b_) { return b_ == static_cast<_::byte>(k + 1); })) {
								++bad;
							}
						}
					}
				);
			}
			for (auto& thread : threads) {
				thread.join();
			}

			EXPECT_EQ(blob.size(), static_cast<streamsize>(slots * slotSize));
		}

		EXPECT_EQ(bad.load(), 0uz);

		std::ifstream in { path, std::ios::binary };
		std::string content { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };
		ASSERT_EQ(content.size(), slots * slotSize);
		for (size_t k = 0; k < slots; ++k) {
			EXPECT_TRUE(
				std::ranges::all_of(
					content.begin() + k * slotSize,
					content.begin() + (k + 1) * slotSize,
					[k](char c_) { return static_cast<unsigned char>(c_) == k + 1; }
				)
			) << "slot " << k;
		}
	}
}
