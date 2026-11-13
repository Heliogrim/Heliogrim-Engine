#include "pch.h"

/**/

#include <atomic>
#include <filesystem>
#include <limits>
#include <fstream>
#include <semaphore>
#include <cstring>
#include <span>
#include <string>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.IO.Dispatcher/IoSystem.hpp>
#include <Engine.Resource.Archive/ArchiveType.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>
#include <Engine.Storage.Action/Access/AccessError.hpp>
#include <Engine.Storage.Boot/Action/WindowedArchive.hpp>
#include <Engine.Storage.Boot/Action/ActionResolverInit.hpp>
#include <Engine.Storage.Registry/Options/ArchiveStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/FileStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/PackageStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Provider/LocalFileSystemProvider.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.Registry/Repository/CacheFileSystemRepository.hpp>
#include <Engine.Storage.Registry/Storage/ArchiveStorage.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>
#include <Engine.Storage.Registry/Url/ArchiveUrl.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Engine.Storage.Registry/Url/PackageUrl.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

#include "Scratch.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

/**/

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

namespace StorageModule {
	class AsyncStorageTest :
		public testing::Test {
	public:
		AsyncStorageTest() {
			Config cfg {};
			_registry.setup(cfg);
			initActionResolver(_system, _resolver);
			_io.start();
			_system.setIo(&_io);

			auto lfsOwned = make_uptr<LocalFileSystemProvider>();
			_lfs = lfsOwned.get();
			_registry.addProvider(::hg::move(lfsOwned));
			_registry.addRepository(_lfs->makeCacheRepository(hg::fs::Path { _scratch.path }));
		}

	public:
		// Note: Destroyed in reverse: the system releases its cached files and queues before the io system and the scratch go.
		ScratchDir _scratch {};
		io::IoSystem _io {};
		StorageRegistry _registry {};
		StorageActionResolver _resolver {};
		StorageSystem _system { _registry, _resolver };
		ptr<LocalFileSystemProvider> _lfs = nullptr;

		constexpr static size_t kSizeA = 1024uz;
		constexpr static size_t kSizeB = 10uz * 1024uz * 1024uz;

		ArchiveGuid _guidA = ArchiveGuid::random();
		ArchiveGuid _guidB = ArchiveGuid::random();
		Arci<PackageStorage> _pack;

	public:
		[[nodiscard]] Arci<IStorage> fileStore(ref<const std::filesystem::path> path_) {
			return _registry.insert(FileStorageDescriptor { FileUrl { "file+cache"sv, hg::fs::Path { path_ } } });
		}

		[[nodiscard]] static std::string pattern(const size_t size_, const char seed_) {
			auto text = std::string(size_, '\0');
			for (size_t i = 0; i < size_; ++i) {
				text[i] = static_cast<char>(seed_ + static_cast<char>(i % 61uz));
			}
			return text;
		}

		void makePackage() {
			auto guid = Guid {};
			GuidGenerate(guid);

			auto file = fileStore(_scratch.path / "pack.imp");
			ASSERT_NE(file, nullptr);
			_pack = _registry.insert(PackageStorageDescriptor { PackageUrl { clone(guid) }, ::hg::move(file) }).into<PackageStorage>();
			ASSERT_NE(_pack, nullptr);

			linkArchive(_guidA, pattern(kSizeA, 'a'));
			linkArchive(_guidB, pattern(kSizeB, 'A'));
		}

		void linkArchive(ref<const ArchiveGuid> guid_, ref<const std::string> data_) {
			auto buffer = BufferArchive {};
			buffer.reserve(data_.size());
			buffer.serializeBytes(const_cast<ptr<char>>(data_.data()), data_.size(), ArchiveStreamMode::eStore);
			buffer.setType(ArchiveType::eRaw);

			const auto mutated = _system.mutate(
				clone(_pack),
				[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
					auto& linked = rwp_.getLinker().add({ .type = ArchiveType::eUndefined, .guid = guid_ });
					linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = streamoff {}, .sizedData = buffer });
					return true;
				}
			);
			ASSERT_TRUE(mutated.has_value());
		}

		[[nodiscard]] Arci<ArchiveStorage> archive(ref<const ArchiveGuid> guid_) {
			return _registry.insert(ArchiveStorageDescriptor { ArchiveUrl { guid_ }, clone(_pack).into<IStorage>() }).into<
				ArchiveStorage>();
		}

		[[nodiscard]] std::string readArchive(ref<const ArchiveGuid> guid_) {
			auto bytes = std::string {};
			const auto result = _system.query(
				archive(guid_),
				[&](_In_ ref<StorageReadonlyArchive> arch_) {
					bytes.resize(static_cast<size_t>(arch_.totalSize()));
					arch_.seek(0);
					arch_.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eLoad);
				}
			);
			EXPECT_TRUE(result.has_value());
			return bytes;
		}
	};

	TEST(AsyncStorageWindow, FitsOnlyInsideSource) {
		EXPECT_TRUE(windowFits(16LL, 0LL, 16LL));
		EXPECT_TRUE(windowFits(16LL, 16LL, 0LL));
		EXPECT_FALSE(windowFits(16LL, 8LL, 9LL));
		EXPECT_FALSE(windowFits(16LL, 17LL, 0LL));
		EXPECT_FALSE(windowFits(16LL, -1LL, 4LL));
		EXPECT_FALSE(windowFits(16LL, 4LL, -1LL));
		// Note: base + size would overflow streamoff; the check must not.
		EXPECT_FALSE(windowFits(16LL, 8LL, std::numeric_limits<streamsize>::max()));
	}

	TEST_F(AsyncStorageTest, SyncQueryOverIoThread) {
		makePackage();
		const auto before = _io.requestsIssued();
		EXPECT_EQ(readArchive(_guidA), pattern(kSizeA, 'a'));
		EXPECT_GT(_io.requestsIssued(), before);
	}

	TEST_F(AsyncStorageTest, LargeArchiveWalks) {
		makePackage();
		// Note: Warm-up caches the package stage, so the walk below only reads archive bytes.
		EXPECT_EQ(readArchive(_guidA), pattern(kSizeA, 'a'));

		const auto before = _io.requestsIssued();
		EXPECT_EQ(readArchive(_guidB), pattern(kSizeB, 'A'));
		// Note: 10 MiB over 8 MiB windows.
		EXPECT_EQ(_io.requestsIssued() - before, 2uLL);
	}

	TEST_F(AsyncStorageTest, MissingFileIsAccessError) {
		const auto path = _scratch.path / "missing.bin";
		auto store = fileStore(path);
		ASSERT_NE(store, nullptr);

		auto ran = false;
		const auto result = _system.query(clone(store), [&](_In_ ref<const Blob>) { ran = true; });
		ASSERT_FALSE(result.has_value());
		EXPECT_TRUE(result.error().is<AccessError>());
		EXPECT_FALSE(ran);
		EXPECT_FALSE(std::filesystem::exists(path));
	}

	TEST_F(AsyncStorageTest, CachedFileAllowsPosixReplace) {
		const auto path = _scratch.path / "replace.bin";
		std::ofstream { path, std::ios::binary } << "old";
		auto store = fileStore(path);
		ASSERT_NE(store, nullptr);

		const auto readAll = [&]() {
			auto bytes = std::string {};
			const auto result = _system.query(
				clone(store),
				[&](_In_ ref<const Blob> blob_) {
					bytes.resize(static_cast<size_t>(blob_.size()));
					std::ignore = blob_.read(streamoff {}, std::span { reinterpret_cast<ptr<_::byte>>(bytes.data()), bytes.size() });
				}
			);
			EXPECT_TRUE(result.has_value());
			return bytes;
		};
		EXPECT_EQ(readAll(), "old");

		const auto tmp = _scratch.path / "replace.tmp";
		std::ofstream { tmp, std::ios::binary } << "new";

		// Note: Legacy replace fails only while the cached handle is open; this proves the cache holds it.
		const auto legacy = _scratch.path / "replace.legacy";
		std::ofstream { legacy, std::ios::binary } << "legacy";
		EXPECT_FALSE(::MoveFileExW(legacy.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING));
		EXPECT_EQ(::GetLastError(), static_cast<DWORD>(ERROR_ACCESS_DENIED));

		const auto handle = ::CreateFileW(tmp.c_str(), DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
		ASSERT_NE(handle, INVALID_HANDLE_VALUE) << ::GetLastError();

		const auto name = path.native();
		const auto nameBytes = name.size() * sizeof(wchar_t);
		auto buffer = Vector<std::byte>(sizeof(FILE_RENAME_INFO) + nameBytes);
		const auto info = reinterpret_cast<ptr<FILE_RENAME_INFO>>(buffer.data());
		info->Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS;
		info->RootDirectory = nullptr;
		info->FileNameLength = static_cast<DWORD>(nameBytes);
		std::memcpy(info->FileName, name.c_str(), nameBytes);

		const auto renamed = ::SetFileInformationByHandle(handle, FileRenameInfoEx, info, static_cast<DWORD>(buffer.size()));
		const auto error = ::GetLastError();
		::CloseHandle(handle);
		ASSERT_TRUE(renamed) << error;
		// Note: No new-bytes check: nothing invalidates the cached stage on external change, so a fresh query still reads the old file.
	}

	TEST_F(AsyncStorageTest, QueryAfterStopIoIsSync) {
		const auto path = _scratch.path / "stop.bin";
		std::ofstream { path, std::ios::binary } << "bytes";
		auto store = fileStore(path);
		ASSERT_NE(store, nullptr);

		const auto readAll = [&]() {
			auto bytes = std::string {};
			const auto result = _system.query(
				clone(store),
				[&](_In_ ref<const Blob> blob_) {
					bytes.resize(static_cast<size_t>(blob_.size()));
					std::ignore = blob_.read(streamoff {}, std::span { reinterpret_cast<ptr<_::byte>>(bytes.data()), bytes.size() });
				}
			);
			EXPECT_TRUE(result.has_value());
			return bytes;
		};

		const auto before = _io.requestsIssued();
		EXPECT_EQ(readAll(), "bytes");
		EXPECT_GT(_io.requestsIssued(), before);

		// Note: Same order as StorageModule::stopIo.
		_io.stop();
		_system.detachIo();

		const auto stopped = _io.requestsIssued();
		EXPECT_EQ(readAll(), "bytes");
		EXPECT_EQ(_io.requestsIssued(), stopped);
	}

	TEST_F(AsyncStorageTest, QueryAsyncReadsArchive) {
		makePackage();
		SchedulerModule::structuredRuntimeTest(
			[&](nmpt<Scheduler>) {
				auto bytes = std::string {};
				auto future = _system.queryAsync(
					archive(_guidA),
					[&](_In_ ref<StorageReadonlyArchive> arch_) {
						bytes.resize(static_cast<size_t>(arch_.totalSize()));
						arch_.seek(0);
						arch_.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eLoad);
					}
				);
				const auto result = await(::hg::move(future));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(bytes, pattern(kSizeA, 'a'));
			}
		);
	}

	TEST_F(AsyncStorageTest, MutateAsyncCommits) {
		makePackage();
		SchedulerModule::structuredRuntimeTest(
			[&](nmpt<Scheduler>) {
				const auto guidC = ArchiveGuid::random();
				const auto data = pattern(64uz, 'c');
				auto buffer = BufferArchive {};
				buffer.reserve(data.size());
				buffer.serializeBytes(const_cast<ptr<char>>(data.data()), data.size(), ArchiveStreamMode::eStore);
				buffer.setType(ArchiveType::eRaw);

				auto added = _system.mutateAsync(
					clone(_pack),
					[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
						auto& linked = rwp_.getLinker().add({ .type = ArchiveType::eUndefined, .guid = guidC });
						linked.changes.emplace_back(package::ArchiveDeltaAdd { .where = streamoff {}, .sizedData = buffer });
						return true;
					}
				);
				ASSERT_TRUE(await(::hg::move(added)).has_value());

				auto bytes = std::string {};
				auto read = _system.queryAsync(
					archive(guidC),
					[&](_In_ ref<StorageReadonlyArchive> arch_) {
						bytes.resize(static_cast<size_t>(arch_.totalSize()));
						arch_.seek(0);
						arch_.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eLoad);
					}
				);
				ASSERT_TRUE(await(::hg::move(read)).has_value());
				EXPECT_EQ(bytes, data);
			}
		);
	}

	TEST_F(AsyncStorageTest, ConcurrentQueryAsync) {
		makePackage();
		SchedulerModule::structuredRuntimeTest(
			[&](nmpt<Scheduler>) {
				std::atomic<u32> okCount { 0 };
				Vector<Future<void, query_archive_error>> pending {};
				for (u32 i = 0; i < 100u; ++i) {
					const auto isA = (i % 2u) == 0u;
					pending.emplace_back(
						_system.queryAsync(
							archive(isA ? _guidA : _guidB),
							[&okCount, isA](_In_ ref<StorageReadonlyArchive> arch_) {
								const auto size = isA ? kSizeA : kSizeB;
								if (static_cast<size_t>(arch_.totalSize()) == size) {
									okCount.fetch_add(1u);
								}
							}
						)
					);
				}
				for (auto& future : pending) {
					EXPECT_TRUE(await(::hg::move(future)).has_value());
				}
				EXPECT_EQ(okCount.load(), 100u);
			}
		);
	}

	#ifdef _DEBUG
	TEST_F(AsyncStorageTest, AffineCallAsserts) {
		const auto path = _scratch.path / "affine.bin";
		std::ofstream { path, std::ios::binary } << "affine";
		auto store = fileStore(path);
		ASSERT_NE(store, nullptr);

		const auto probe = [&]() {
				std::counting_semaphore<> wake { 0 };
				async::AffineThread thread { "affine-probe", [&wake]() { wake.release(); } };
				thread.start(
					[&wake](ref<async::AffineThread> self_) {
						for (;;) {
							self_.drain();
							if (self_.stopping()) {
								self_.drain();
								return;
							}
							wake.acquire();
						}
					}
				);
				auto done = thread.post([&]() { std::ignore = _system.query(clone(store), [](_In_ ref<const Blob>) {}); });
				std::ignore = await(::hg::move(done));
				thread.stop();
		};
		EXPECT_DEATH(probe(), "");
	}
	#endif
}
