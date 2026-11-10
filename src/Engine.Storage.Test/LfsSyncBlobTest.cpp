#include "pch.h"

/**/

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <thread>
#include <vector>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>

/**/

using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	[[nodiscard]] static std::filesystem::path uniqueTempFile() {
		return std::filesystem::temp_directory_path() / std::format(
			"hg-lfs-sync-blob-{}.bin",
			std::chrono::steady_clock::now().time_since_epoch().count()
		);
	}

	TEST(LfsSyncBlobTest, ConcurrentReadsReturnOwnRanges) {

		const auto path = uniqueTempFile();
		std::filesystem::remove(path);

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

		std::filesystem::remove(path);
		EXPECT_EQ(mismatches.load(), 0uz);
	}

	TEST(LfsSyncBlobTest, ReadOnlyFileReads) {

		const auto path = uniqueTempFile(); {
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
		std::filesystem::remove(path);

		EXPECT_EQ(std::string_view(reinterpret_cast<const char*>(buf.data()), size), "read only");
	}
}
