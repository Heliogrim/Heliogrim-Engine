#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
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
		Guid _packGuid {};

	protected:
		void SetUp() override {
			auto memGuid = Guid {};
			GuidGenerate(memGuid);
			auto packGuid = Guid {};
			GuidGenerate(packGuid);
			_packGuid = packGuid;

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

		/* Standalone memory storage; nothing refers to it, so it can be removed. */
		[[nodiscard]] Arci<IStorage> makeMemory(cref<Guid> guid_) {
			return _registry.insert(MemoryStorageDescriptor { MemoryUrl { clone(guid_) }, 4096uLL, MemoryProperty::eUndefined });
		}

		[[nodiscard]] static Guid freshGuid() {
			auto guid = Guid {};
			GuidGenerate(guid);
			return guid;
		}

		/* Always exactly 8 bytes, so versions never change the blob layout. */
		[[nodiscard]] bool writeText(cref<Arci<IStorage>> store_, StringView text_) {
			return _system.mutate(
				clone(store_),
				[text_](_In_ ref<Blob> blob_) {
					blob_.write(streamoff {}, std::span { std::bit_cast<_::byte*>(const_cast<char*>(text_.data())), text_.size() });
					return true;
				}
			).has_value();
		}

		[[nodiscard]] std::string readText(cref<Arci<IStorage>> store_, ptr<bool> ran_ = nullptr) {
			auto text = std::string(8uLL, '\0');
			const auto result = _system.query(
				clone(store_),
				[&](_In_ cref<Blob> blob_) {
					if (ran_) {
						*ran_ = true;
					}
					blob_.read(streamoff {}, std::span { std::bit_cast<_::byte*>(text.data()), text.size() });
				}
			);
			if (not result.has_value()) {
				return {};
			}
			return text;
		}

		[[nodiscard]] static std::string rawText(cref<Arci<IStorage>> store_) {
			auto span = clone(store_).into<system::MemoryStorage>()->_memory.span();
			return std::string { std::bit_cast<const char*>(span.data()), 8uLL };
		}

		template <typename Fn_>
		static bool waitUntil(Fn_&& pred_, std::chrono::milliseconds limit_ = std::chrono::seconds { 10 }) {
			const auto end = std::chrono::steady_clock::now() + limit_;
			while (not pred_()) {
				if (std::chrono::steady_clock::now() > end) {
					return false;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds { 1 });
			}
			return true;
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

	TEST_F(StackCacheIntegration, OverlappingQueriesGetOwnArchiveCursor) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "v0000"sv));

		ptr<const void> outer = nullptr;
		ptr<const void> inner = nullptr;
		s64 outerCursorBefore = -1;
		s64 outerCursorAfter = -1;
		std::string innerRead {};

		// Note: Fails if the archive stage is shared: the inner read would move the outer cursor, and both see one object.
		ASSERT_TRUE(
			_system.query(
				clone(storeA),
				[&](_In_ ref<StorageReadonlyArchive> outer_) {
					outer = std::addressof(outer_);
					outerCursorBefore = outer_.tell();
					EXPECT_TRUE(
						_system.query(
							clone(storeA),
							[&](_In_ ref<StorageReadonlyArchive> inner_) {
								inner = std::addressof(inner_);
								innerRead = readAll(inner_);
							}
						).has_value()
					);
					outerCursorAfter = outer_.tell();
				}
			).has_value()
		);

		EXPECT_NE(outer, nullptr);
		EXPECT_NE(inner, nullptr);
		EXPECT_NE(outer, inner);
		EXPECT_EQ(innerRead, "v0000");
		EXPECT_EQ(outerCursorBefore, outerCursorAfter);
	}

	TEST_F(StackCacheIntegration, ConcurrentReadersSeeConsistentContent) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "v0000"sv));

		// Note: Fails on any shared per-reader state in a cached stage: interleaved cursors give wrong or short reads.
		constexpr auto readerCount = 6;
		constexpr auto readsPerReader = 400;
		std::atomic_int failed = 0;
		std::atomic_int reads = 0;

		std::vector<std::thread> readers {};
		for (auto idx = 0; idx < readerCount; ++idx) {
			readers.emplace_back(
				[&] {
					for (auto iter = 0; iter < readsPerReader; ++iter) {
						std::string read {};
						const auto result = _system.query(
							clone(storeA),
							[&](_In_ ref<StorageReadonlyArchive> arch_) { read = readAll(arch_); }
						);
						if (not result.has_value() || read != "v0000") {
							++failed;
							continue;
						}
						++reads;
					}
				}
			);
		}
		for (auto& reader : readers) {
			reader.join();
		}

		EXPECT_EQ(failed.load(), 0);
		EXPECT_EQ(reads.load(), readerCount * readsPerReader);
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
		auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
		auto* const lfs = lfsOwned.get();
		_registry.addProvider(::hg::move(lfsOwned));
		_registry.addRepository(lfs->makeCacheRepository(fs::Path { _scratch.path }));

		// Note: The storage path is the url path as given, so an absolute scratch path keeps the file out of the cwd.
		auto fileUrl = FileUrl { "file+cache"sv, fs::Path { _scratch.path / "stack-cache.bin" } };

		auto store = _registry.insert(FileStorageDescriptor { clone(fileUrl) });
		ASSERT_NE(store, nullptr);
		const auto path = static_cast<std::filesystem::path>(static_cast<ref<const system::LocalFileStorage>>(*store)._lfsPath);
		std::ofstream { path, std::ios::binary } << "Stack Cache Blob";

		ASSERT_TRUE(_system.query(clone(store), [](_In_ ref<const Blob>) {}).has_value());
		const auto key = StageKey { store.get(), refl::TypeId<Rc<IoResourceAccessor<const LfsSyncBlob>>>() };
		ASSERT_TRUE(stacks().find(key).has_value());

		ASSERT_TRUE(_system.removeStorage(clone(fileUrl)).has_value());

		EXPECT_FALSE(stacks().find(key).has_value());
		auto ec = std::error_code {};
		EXPECT_TRUE(std::filesystem::remove(path, ec)) << ec.message();
	}

	/* Bad paths: removal, retirement */

	TEST_F(StackCacheIntegration, QueryThroughStaleArciAfterRemovalIsRejected) {
		const auto guid = freshGuid();
		auto mem = makeMemory(guid);
		ASSERT_NE(mem, nullptr);
		ASSERT_TRUE(writeText(mem, "abcdefgh"sv));
		EXPECT_EQ(readText(mem), "abcdefgh");

		auto stale = clone(mem);
		ASSERT_TRUE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());
		EXPECT_TRUE(mem->isRetired());

		const auto size = cacheSize();
		const auto attempts = insertAttempts();
		auto ran = false;
		const auto result = _system.query(clone(stale), [&](_In_ cref<Blob>) { ran = true; });

		ASSERT_FALSE(result.has_value());
		EXPECT_TRUE(result.error().is<AccessError>());
		EXPECT_FALSE(ran);
		EXPECT_EQ(cacheSize(), size);
		EXPECT_EQ(insertAttempts(), attempts);
	}

	TEST_F(StackCacheIntegration, MutationOnRemovedStorageIsRejected) {
		const auto guid = freshGuid();
		auto mem = makeMemory(guid);
		ASSERT_NE(mem, nullptr);
		ASSERT_TRUE(writeText(mem, "original"sv));
		ASSERT_TRUE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());

		const auto size = cacheSize();
		const auto attempts = insertAttempts();
		auto ran = false;
		const auto result = _system.mutate(
			clone(mem),
			[&](_In_ ref<Blob>) {
				ran = true;
				return true;
			}
		);

		ASSERT_FALSE(result.has_value());
		EXPECT_TRUE(result.error().is<AccessError>());
		EXPECT_FALSE(ran);
		EXPECT_EQ(rawText(mem), "original");
		EXPECT_EQ(cacheSize(), size);
		EXPECT_EQ(insertAttempts(), attempts);
	}

	TEST_F(StackCacheIntegration, ValueQueryOnRemovedStorageIsRejected) {
		const auto guid = freshGuid();
		auto mem = makeMemory(guid);
		ASSERT_NE(mem, nullptr);
		ASSERT_TRUE(writeText(mem, "abcdefgh"sv));
		ASSERT_TRUE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());

		const auto size = cacheSize();
		const auto attempts = insertAttempts();
		auto ran = false;
		// Note: A non-void callback routes through the value-returning `execQuery`.
		const auto result = _system.query(
			clone(mem),
			[&](_In_ cref<Blob>) {
				ran = true;
				return 1;
			}
		);

		ASSERT_FALSE(result.has_value());
		EXPECT_TRUE(result.error().is<AccessError>());
		EXPECT_FALSE(ran);
		EXPECT_EQ(cacheSize(), size);
		EXPECT_EQ(insertAttempts(), attempts);
	}

	TEST_F(StackCacheIntegration, RemoveStorageOfUnknownOrWrongKindOrRepeatedReturnsNone) {
		const auto guid = freshGuid();
		auto mem = makeMemory(guid);
		ASSERT_NE(mem, nullptr);
		ASSERT_TRUE(writeText(mem, "abcdefgh"sv));
		const auto size = cacheSize();
		const auto attempts = insertAttempts();

		// unknown memory url
		EXPECT_FALSE(_system.removeStorage(MemoryUrl { freshGuid() }).has_value());
		// Note: a package's guid under the memory scheme maps to no memory storage; the package stays.
		EXPECT_FALSE(_system.removeStorage(MemoryUrl { clone(_packGuid) }).has_value());
		// unknown file url (no repository for the scheme)
		EXPECT_FALSE(_system.removeStorage(FileUrl { "file+nowhere"sv, fs::Path { "absent.bin" } }).has_value());

		EXPECT_EQ(cacheSize(), size);
		EXPECT_EQ(insertAttempts(), attempts);
		EXPECT_FALSE(mem->isRetired());
		EXPECT_FALSE(_pack->isRetired());
		EXPECT_EQ(readText(mem), "abcdefgh");
		EXPECT_TRUE(_system.query(clone(_pack), [](_In_ cref<ReadonlyPackage>) {}).has_value());

		ASSERT_TRUE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());
		const auto afterRemove = cacheSize();
		EXPECT_FALSE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());
		EXPECT_EQ(cacheSize(), afterRemove);
		EXPECT_TRUE(mem->isRetired());
	}

	TEST_F(StackCacheIntegration, RemovalWaitsForInFlightQuery) {
		const auto guid = freshGuid();
		auto mem = makeMemory(guid);
		ASSERT_NE(mem, nullptr);
		ASSERT_TRUE(writeText(mem, "abcdefgh"sv));

		std::atomic_bool entered = false;
		std::atomic_bool release = false;
		std::atomic_bool removed = false;
		std::atomic_bool querySucceeded = false;
		std::atomic_bool removeSome = false;

		// Note: A regression (removal not taking the unique lock) lets `removed` flip while the query is still blocked.
		std::thread reader {
			[&] {
				const auto result = _system.query(
					clone(mem),
					[&](_In_ cref<Blob>) {
						entered = true;
						waitUntil([&] { return release.load(); });
					}
				);
				querySucceeded = result.has_value();
			}
		};
		const auto readerEntered = waitUntil([&] { return entered.load(); });
		if (not readerEntered) {
			release = true;
			reader.join();
			FAIL() << "query never entered";
		}

		std::atomic_bool removerEntered = false;
		std::thread remover {
			[&] {
				removerEntered = true;
				removeSome = _system.removeStorage(MemoryUrl { clone(guid) }).has_value();
				removed = true;
			}
		};

		const auto removerStarted = waitUntil([&] { return removerEntered.load(); });
		// Note: No hook into the mutex wait queue, so a grace period stands in for "the remover blocks on the unique lock".
		std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
		const auto earlyRemoval = removed.load();
		release = true;
		const auto removedInTime = waitUntil([&] { return removed.load(); });

		reader.join();
		remover.join();

		EXPECT_TRUE(removerStarted);
		EXPECT_FALSE(earlyRemoval);
		EXPECT_TRUE(removedInTime);
		EXPECT_TRUE(querySucceeded.load());
		EXPECT_TRUE(removeSome.load());
		EXPECT_TRUE(mem->isRetired());
		EXPECT_EQ(cacheSize(), 0uLL);
	}

	TEST_F(StackCacheIntegration, ConcurrentRemovalBehindQueryHasExactlyOneLoser) {
		const auto removeBehindQuery = [&](const auto& url_, const auto& storage_) {
			std::atomic_bool queryEntered = false;
			std::atomic_bool release = false;
			std::atomic_int removersEntered = 0;
			std::atomic_int some = 0;
			std::atomic_int none = 0;

			std::thread reader {
				[&] {
					const auto result = _system.query(
						clone(storage_),
						[&](_In_ ref<const Blob>) {
							queryEntered = true;
							waitUntil([&] { return release.load(); });
						}
					);
					EXPECT_TRUE(result.has_value());
				}
			};
			if (not waitUntil([&] { return queryEntered.load(); })) {
				release = true;
				reader.join();
				FAIL() << "query never entered";
			}

			std::vector<std::thread> removers {};
			for (auto idx = 0; idx < 2; ++idx) {
				removers.emplace_back(
					[&] {
						++removersEntered;
						if (_system.removeStorage(clone(url_)).has_value()) {
							++some;
						} else {
							++none;
						}
					}
				);
			}

			const auto bothEntered = waitUntil([&] { return removersEntered.load() == 2; });
			// Note: No hook into the mutex wait queue, so a grace period stands in for "both removers block on the unique lock".
			std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
			const auto early = some.load() + none.load();
			release = true;
			reader.join();
			for (auto& remover : removers) {
				remover.join();
			}

			EXPECT_TRUE(bothEntered);
			EXPECT_EQ(early, 0);
			EXPECT_EQ(some.load(), 1);
			EXPECT_EQ(none.load(), 1);
			EXPECT_TRUE(storage_->isRetired());
			EXPECT_EQ(cacheSize(), 0uLL);
		};

		{
			const auto guid = freshGuid();
			auto mem = makeMemory(guid);
			ASSERT_NE(mem, nullptr);
			ASSERT_TRUE(writeText(mem, "abcdefgh"sv));
			removeBehindQuery(MemoryUrl { clone(guid) }, mem);
		}

		auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
		auto* const lfs = lfsOwned.get();
		_registry.addProvider(::hg::move(lfsOwned));
		_registry.addRepository(lfs->makeCacheRepository(fs::Path { _scratch.path }));

		const auto path = _scratch.path / "stack-cache-race.bin";
		std::ofstream { path, std::ios::binary } << "Stack Cache Blob";

		auto fileUrl = FileUrl { "file+cache"sv, fs::Path { path } };
		auto store = _registry.insert(FileStorageDescriptor { clone(fileUrl) });
		ASSERT_NE(store, nullptr);
		removeBehindQuery(fileUrl, store);
	}

	TEST_F(StackCacheIntegration, ConcurrentRemovalOfSameUrlYieldsExactlyOneSome) {
		for (auto round = 0; round < 10; ++round) {
			const auto guid = freshGuid();
			auto mem = makeMemory(guid);
			ASSERT_NE(mem, nullptr);
			ASSERT_TRUE(writeText(mem, "abcdefgh"sv));

			std::atomic_bool go = false;
			std::atomic_int some = 0;
			std::atomic_int none = 0;
			std::vector<std::thread> threads {};
			for (auto idx = 0; idx < 2; ++idx) {
				threads.emplace_back(
					[&] {
						waitUntil([&] { return go.load(); });
						if (_system.removeStorage(MemoryUrl { clone(guid) }).has_value()) {
							++some;
						} else {
							++none;
						}
					}
				);
			}
			go = true;
			for (auto& thread : threads) {
				thread.join();
			}

			EXPECT_EQ(some.load(), 1);
			EXPECT_EQ(none.load(), 1);
			EXPECT_TRUE(mem->isRetired());
		}
	}

	TEST_F(StackCacheIntegration, ReRegisterAfterRemovalIsNotServedFromOldStages) {
		const auto guid = freshGuid();
		auto oldMem = makeMemory(guid);
		ASSERT_NE(oldMem, nullptr);
		ASSERT_TRUE(writeText(oldMem, "old-data"sv));
		EXPECT_EQ(readText(oldMem), "old-data");
		ASSERT_TRUE(_system.removeStorage(MemoryUrl { clone(guid) }).has_value());

		auto newMem = makeMemory(guid);
		ASSERT_NE(newMem, nullptr);
		EXPECT_NE(newMem.get(), oldMem.get());
		ASSERT_TRUE(writeText(newMem, "new-data"sv));

		const auto attempts = insertAttempts();
		EXPECT_EQ(readText(newMem), "new-data");
		EXPECT_GT(insertAttempts(), attempts);

		// Note: The old handle stays rejected, the new one is not retired.
		EXPECT_TRUE(oldMem->isRetired());
		EXPECT_FALSE(newMem->isRetired());
		EXPECT_FALSE(_system.query(clone(oldMem), [](_In_ cref<Blob>) {}).has_value());
	}

	/* Bad paths: failed builds and mutations */

	TEST_F(StackCacheIntegration, FailedQueryBuildKeepsLowerStagesReusable) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		const auto size = cacheSize();
		const auto attempts = insertAttempts();

		auto missing = archive(ArchiveGuid::random());
		ASSERT_NE(missing, nullptr);
		auto ran = false;
		EXPECT_FALSE(_system.query(clone(missing), [&](_In_ ref<StorageReadonlyArchive>) { ran = true; }).has_value());
		EXPECT_FALSE(ran);
		// Note: Blob and package were cached already, the failed archive stage adds nothing.
		EXPECT_EQ(cacheSize(), size);
		EXPECT_EQ(insertAttempts(), attempts);

		const auto beforeReuse = insertAttempts();
		EXPECT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		EXPECT_EQ(insertAttempts(), beforeReuse);
	}

	TEST_F(StackCacheIntegration, MutationCallbackReturningFalseInvalidatesAndKeepsData) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		ASSERT_TRUE(_system.query(clone(storeA), [](_In_ ref<StorageReadonlyArchive>) {}).has_value());
		ASSERT_GT(cacheSize(), 0uLL);

		// Note: Without staged changes nothing is committed, so the data stays; the cache must still be dropped.
		const auto noChange = _system.mutate(clone(_pack), [](_Inout_ ref<ReadWritePackage>) -> bool { return false; });
		EXPECT_TRUE(noChange.has_value());
		EXPECT_EQ(cacheSize(), 0uLL);

		std::string read {};
		const auto attempts = insertAttempts();
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { read = readAll(arch_); }).has_value());
		EXPECT_EQ(read, "Stack Cache Data");
		EXPECT_GT(insertAttempts(), attempts);

		// Note: Contract by release stage: staged changes are committed whatever the callback returns.
		auto buffer = BufferArchive {};
		constexpr auto text = StringView { "Rejected Data"sv };
		buffer.reserve(text.size());
		buffer.serializeBytes(const_cast<char*>(text.data()), text.size(), ArchiveStreamMode::eStore);
		buffer.setType(ArchiveType::eRaw);
		const auto guidA = _guidA;
		const auto rejecting = _system.mutate(
			clone(_pack),
			[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
				auto linked = rwp_.getLinker()[guidA];
				linked->changes.emplace_back(
					package::ArchiveDeltaReplace { .where = streamoff {}, .byteCount = linked->data.size, .withSizedData = buffer }
				);
				return false;
			}
		);
		EXPECT_TRUE(rejecting.has_value());
		EXPECT_EQ(cacheSize(), 0uLL);

		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { read = readAll(arch_); }).has_value());
		EXPECT_EQ(read, "Rejected Data");
	}

	TEST_F(StackCacheIntegration, MutationOnOnePrimaryLeavesUnrelatedChainsIntact) {
		const auto guid = freshGuid();
		auto other = makeMemory(guid);
		ASSERT_NE(other, nullptr);
		ASSERT_TRUE(writeText(other, "abcdefgh"sv));
		EXPECT_EQ(readText(other), "abcdefgh");
		const auto size = cacheSize();
		ASSERT_GT(size, 0uLL);

		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "Other Data"sv));

		// Note: The mutation of the package primary adds and drops nothing of `other`.
		EXPECT_EQ(cacheSize(), size);
		const auto attempts = insertAttempts();
		EXPECT_EQ(readText(other), "abcdefgh");
		EXPECT_EQ(insertAttempts(), attempts);
		EXPECT_EQ(cacheSize(), size);
	}

	TEST_F(StackCacheIntegration, MutationOfArchiveAGivesFreshPackageToArchiveB) {
		auto storeA = archive(_guidA);
		auto storeB = archive(_guidB);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NE(storeB, nullptr);

		std::string readB {};
		ASSERT_TRUE(_system.query(clone(storeB), [&](_In_ ref<StorageReadonlyArchive> arch_) { readB = readAll(arch_); }).has_value());
		EXPECT_EQ(readB, "Stack Cache Data");

		const auto packBefore = packageAddress();
		// Note: Pins the old package node, so a rebuilt package cannot reuse its address.
		const auto pinned = stacks().find(StageKey { _pack.get(), refl::TypeId<Rc<IoResourceAccessor<const ReadonlyPackage>>>() });
		ASSERT_TRUE(pinned.has_value());

		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "A changed!"sv));

		const auto attempts = insertAttempts();
		std::string afterB {};
		ASSERT_TRUE(_system.query(clone(storeB), [&](_In_ ref<StorageReadonlyArchive> arch_) { afterB = readAll(arch_); }).has_value());
		EXPECT_EQ(afterB, "Stack Cache Data");
		EXPECT_GT(insertAttempts(), attempts);
		EXPECT_NE(packBefore, packageAddress());

		std::string afterA {};
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { afterA = readAll(arch_); }).has_value());
		EXPECT_EQ(afterA, "A changed!");
	}

	TEST_F(StackCacheIntegration, ReadersNeverSeeVersionOlderThanCommittedMutation) {
		auto storeA = archive(_guidA);
		ASSERT_NE(storeA, nullptr);
		// Note: Versions have a fixed length ("v0000".."v9999"), so the archive layout never changes.
		ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, "v0000"sv));
		{
			std::string first {};
			ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { first = readAll(arch_); }).has_value());
			ASSERT_EQ(first, "v0000");
		}

		constexpr auto versions = 60;
		std::atomic_int committed = 0;
		std::atomic_bool done = false;
		std::atomic_int stale = 0;
		std::atomic_int failed = 0;
		std::atomic_int reads = 0;

		// Note: Fails on a regression that serves a pre-commit stage or lets a reader overlap the commit
		// Note: (stale version, torn or unparsable text, or a failed query).
		std::vector<std::thread> readers {};
		for (auto idx = 0; idx < 4; ++idx) {
			readers.emplace_back(
				[&] {
					while (not done.load()) {
						const auto floor = committed.load();
						std::string read {};
						const auto result = _system.query(
							clone(storeA),
							[&](_In_ ref<StorageReadonlyArchive> arch_) { read = readAll(arch_); }
						);
						if (not result.has_value() || read.size() != 5 || read[0] != 'v') {
							++failed;
							continue;
						}
						++reads;
						if (std::stoi(read.substr(1)) < floor) {
							++stale;
						}
					}
				}
			);
		}

		for (auto version = 1; version <= versions; ++version) {
			ASSERT_NO_FATAL_FAILURE(rewriteArchive(_guidA, std::format("v{:04}", version)));
			committed = version;
		}
		done = true;
		for (auto& reader : readers) {
			reader.join();
		}

		EXPECT_EQ(stale.load(), 0);
		EXPECT_EQ(failed.load(), 0);
		EXPECT_GT(reads.load(), 0);
	}
}
