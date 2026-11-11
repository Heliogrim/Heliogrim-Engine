#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflect/FullTypeInfo.hpp>
#include <Engine.Resource.Archive/ArchiveType.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Blob/File/LfsSyncBlob.hpp>
#include <Engine.Resource.Package/ReadonlyPackage.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>
#include <Engine.Storage.Registry/Options/ArchiveStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/FileStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Provider/LocalFileSystemProvider.hpp>
#include <Engine.Storage.Registry/Repository/CacheFileSystemRepository.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>
#include <Engine.Storage.Registry/Url/ArchiveUrl.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Engine.Storage.Registry/Url/MemoryUrl.hpp>
#include <Engine.Storage.Registry/Url/StorageUrl.hpp>

/**/

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	struct StackCachePeek : StorageSystem {
		using StorageSystem::_context;
	};

	/* Explicit instantiation skips access checks, which reaches the private `fetchQuery` overload. */
	using fetch_archive_fn = IoQueryGuard<StorageReadonlyArchive> (StorageSystem::*)(mref<Arci<ArchiveStorage>>) const noexcept;

	fetch_archive_fn fetchArchiveFn();

	template <fetch_archive_fn Fn_>
	struct FetchArchiveAccess {
		friend fetch_archive_fn fetchArchiveFn() {
			return Fn_;
		}
	};

	template struct FetchArchiveAccess<&StorageSystem::fetchQuery>;

	class StackCacheIntegration :
		public MutationIntegrationFixture {
	public:
		Arci<PackageStorage> _pack;
		ptr<const IStorage> _mem = nullptr;
		ArchiveGuid _guidA = ArchiveGuid::random();
		ArchiveGuid _guidB = ArchiveGuid::random();

	protected:
		void SetUp() override {
			auto memGuid = Guid {};
			GuidGenerate(memGuid);
			auto packGuid = Guid {};
			GuidGenerate(packGuid);

			auto memStore = _registry.insert(MemoryStorageDescriptor { MemoryUrl { clone(memGuid) }, 4096uLL, MemoryProperty::eUndefined });
			ASSERT_NE(memStore, nullptr);
			_mem = memStore.get();
			_pack = _registry.insert(
				PackageStorageDescriptor { PackageUrl { clone(packGuid) }, ::hg::move(memStore) }
			).into<PackageStorage>();
			ASSERT_NE(_pack, nullptr);

			linkArchive(_guidA);
			linkArchive(_guidB);
		}

		void linkArchive(cref<ArchiveGuid> guid_) {
			constexpr auto data = StringView { "Stack Cache Data"sv };
			auto buffer = BufferArchive {};
			buffer.reserve(data.size());
			buffer.serializeBytes(const_cast<char*>(data.data()), data.size(), ArchiveStreamMode::eStore);
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

		void rewriteArchive(cref<ArchiveGuid> guid_, StringView data_) {
			auto buffer = BufferArchive {};
			buffer.reserve(data_.size());
			buffer.serializeBytes(const_cast<char*>(data_.data()), data_.size(), ArchiveStreamMode::eStore);
			buffer.setType(ArchiveType::eRaw);

			const auto mutated = _system.mutate(
				clone(_pack),
				[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
					auto linked = rwp_.getLinker()[guid_];
					linked->changes.emplace_back(
						package::ArchiveDeltaReplace { .where = streamoff {}, .byteCount = linked->data.size, .withSizedData = buffer }
					);
					return true;
				}
			);
			EXPECT_TRUE(mutated.has_value());
		}

		[[nodiscard]] static std::string readAll(ref<StorageReadonlyArchive> arch_) {
			auto bytes = std::string(static_cast<size_t>(arch_.totalSize()), '\0');
			arch_.seek(0);
			arch_.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eLoad);
			return bytes;
		}

		[[nodiscard]] ref<StackCache> stacks() {
			return (_system.*&StackCachePeek::_context).stacks;
		}

		[[nodiscard]] size_t cacheSize() const {
			return (_system.*&StackCachePeek::_context).stacks.size();
		}

		[[nodiscard]] size_t insertAttempts() const {
			return (_system.*&StackCachePeek::_context).stacks.insertAttempts();
		}

		[[nodiscard]] Arci<ArchiveStorage> archive(cref<ArchiveGuid> guid_) {
			return _registry.insert(ArchiveStorageDescriptor { ArchiveUrl { guid_ }, clone(_pack).into<IStorage>() }).into<
				ArchiveStorage>();
		}

		[[nodiscard]] ptr<const void> packageAddress() const {
			ptr<const void> address = nullptr;
			const auto result = _system.query(clone(_pack), [&](_In_ cref<ReadonlyPackage> rp_) { address = std::addressof(rp_); });
			EXPECT_TRUE(result.has_value());
			return address;
		}
	};

	TEST_F(StackCacheIntegration, RepeatedQueryResolvesOnce) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);

		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		const auto packBefore = packageAddress();
		const auto attempts = insertAttempts();
		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());

		// Note: A rebuild would still return the stored nodes, so only the insert count proves the cache hit.
		// Note: Blob and package are cached; the archive (a cursor resource) is built per query and never inserted.
		EXPECT_EQ(insertAttempts() - attempts, 0uLL);
		EXPECT_EQ(packBefore, packageAddress());
		EXPECT_EQ(cacheSize(), 2uLL);
	}

	TEST_F(StackCacheIntegration, ArchiveReusesPackagePrefix) {
		auto storeA = archive(_guidA);
		auto storeB = archive(_guidB);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NE(storeB, nullptr);

		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		const auto packBefore = packageAddress();
		const auto attempts = insertAttempts();
		ASSERT_TRUE(_system.query(clone(storeB), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());

		// Note: Only the archive stage is built, per query and uncached; blob and package come from the cache.
		EXPECT_EQ(insertAttempts() - attempts, 0uLL);
		EXPECT_EQ(packBefore, packageAddress());
		EXPECT_EQ(cacheSize(), 2uLL);
	}

	TEST_F(StackCacheIntegration, OverlappingQueriesShareStages) {
		auto storeA = archive(_guidA);
		auto storeB = archive(_guidB);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NE(storeB, nullptr);

		bool innerRan = false;
		bool outerRan = false; {
			auto guardA = (_system.*fetchArchiveFn())(clone(storeA));
			EXPECT_TRUE(_system.query(clone(storeB), [&](_In_ ref<StorageReadonlyArchive>) { innerRan = true; }).has_value());
			EXPECT_TRUE(guardA([&](_In_ ref<StorageReadonlyArchive>) { outerRan = true; }).has_value());
		}

		EXPECT_TRUE(innerRan);
		EXPECT_TRUE(outerRan);
		// Note: Blob and package are shared; each query builds its own archive on top.
		EXPECT_EQ(cacheSize(), 2uLL);
	}

	TEST_F(StackCacheIntegration, FailedBuildCachesNothing) {
		auto missing = archive(ArchiveGuid::random());
		ASSERT_NE(missing, nullptr);

		EXPECT_FALSE(_system.query(clone(missing), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		// Note: Blob and package resolved below the failure stay cached, the archive stage does not.
		const auto afterFirst = cacheSize();
		EXPECT_EQ(afterFirst, 2uLL);

		EXPECT_FALSE(_system.query(clone(missing), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		EXPECT_EQ(cacheSize(), afterFirst);
	}

	TEST_F(StackCacheIntegration, MutationInvalidatesAfterCommit) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());

		const auto packBefore = packageAddress();
		// Note: Pins the old package node, so the rebuilt package cannot reuse its address.
		const auto pinned = stacks().find(StageKey { _pack.get(), refl::TypeId<Rc<IoResourceAccessor<const ReadonlyPackage>>>() });
		ASSERT_TRUE(pinned.has_value());

		const auto guidC = ArchiveGuid::random();
		linkArchive(guidC);
		auto storeC = archive(guidC);
		ASSERT_NE(storeC, nullptr);

		EXPECT_TRUE(_system.query(clone(storeC), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		EXPECT_NE(packBefore, packageAddress());
	}

	TEST_F(StackCacheIntegration, QueryAfterMutationSeesCommit) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);

		std::string before {};
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { before = readAll(arch_); }).has_value());
		EXPECT_EQ(before, "Stack Cache Data");

		// Note: A different length, so a stale linker entry would read the wrong size.
		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "Fresh Data"sv));

		std::string after {};
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { after = readAll(arch_); }).has_value());
		EXPECT_EQ(after, "Fresh Data");
	}

	TEST_F(StackCacheIntegration, GuardKeepsInvalidatedChainAlive) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);

		std::string read {};
		size_t sizeInside = ~0uLL;
		auto guardA = (_system.*fetchArchiveFn())(clone(storeA));
		// Note: The guard holds the resolved chain while its callback runs; drop every entry underneath it.
		EXPECT_TRUE(
			guardA(
				[&](_In_ ref<StorageReadonlyArchive> arch_) {
					stacks().invalidateFrom(_mem);
					sizeInside = cacheSize();
					read = readAll(arch_);
				}
			).has_value()
		);

		EXPECT_EQ(sizeInside, 0uLL);
		EXPECT_EQ(read, "Stack Cache Data");
	}

	TEST_F(StackCacheIntegration, RemoveStorageDropsEntries) {
		const auto name = std::format("hg-stack-cache-{}.bin", std::chrono::steady_clock::now().time_since_epoch().count());
		auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
		auto* const lfs = lfsOwned.get();
		_registry.addProvider(::hg::move(lfsOwned));
		_registry.addRepository(lfs->makeCacheRepository(fs::Path { std::filesystem::temp_directory_path() }));

		// Note: The storage path is the url path as given, so an absolute temp path keeps the file out of the cwd.
		auto fileUrl = FileUrl { "file+cache"sv, fs::Path { std::filesystem::temp_directory_path() / name } };

		struct RemoveOnExit {
			std::filesystem::path path;

			~RemoveOnExit() {
				auto ec = std::error_code {};
				std::filesystem::remove(path, ec);
			}
		} cleanup {};// declared before `store`, so the file is removed after the storage is released

		auto store = _registry.insert(FileStorageDescriptor { clone(fileUrl) });
		ASSERT_NE(store, nullptr);
		const auto path = static_cast<std::filesystem::path>(static_cast<ref<const system::LocalFileStorage>>(*store)._lfsPath);
		cleanup.path = path;
		std::ofstream { path, std::ios::binary } << "Stack Cache Blob";

		ASSERT_TRUE(_system.query(clone(store), [](_In_ ref<const Blob>) {}).has_value());
		const auto key = StageKey { store.get(), refl::TypeId<Rc<IoResourceAccessor<const LfsSyncBlob>>>() };
		ASSERT_TRUE(stacks().find(key).has_value());

		ASSERT_TRUE(_system.removeStorage(clone(fileUrl)).has_value());

		EXPECT_FALSE(stacks().find(key).has_value());
		auto ec = std::error_code {};
		EXPECT_TRUE(std::filesystem::remove(path, ec)) << ec.message();
	}
}
