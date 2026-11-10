#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <Engine.Common/Guid.hpp>
#include <Engine.Resource.Archive/ArchiveType.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>
#include <Engine.Storage.Registry/Options/ArchiveStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Url/ArchiveUrl.hpp>
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
			_pack = _registry.insert(PackageStorageDescriptor { PackageUrl { clone(packGuid) }, ::hg::move(memStore) }).into<PackageStorage>();
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

		[[nodiscard]] size_t cacheSize() const {
			return (_system.*&StackCachePeek::_context).stacks.size();
		}

		[[nodiscard]] size_t insertAttempts() const {
			return (_system.*&StackCachePeek::_context).stacks.insertAttempts();
		}

		[[nodiscard]] Arci<ArchiveStorage> archive(cref<ArchiveGuid> guid_) {
			return _registry.insert(ArchiveStorageDescriptor { ArchiveUrl { guid_ }, clone(_pack).into<IStorage>() }).into<ArchiveStorage>();
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

		ptr<const void> first = nullptr;
		ptr<const void> second = nullptr;
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { first = std::addressof(arch_); }).has_value());
		const auto attempts = insertAttempts();
		ASSERT_TRUE(_system.query(clone(storeA), [&](_In_ ref<StorageReadonlyArchive> arch_) { second = std::addressof(arch_); }).has_value());

		EXPECT_NE(first, nullptr);
		EXPECT_EQ(first, second);
		// Note: A rebuild would still return the stored nodes, so only the insert count proves the cache hit.
		EXPECT_EQ(insertAttempts() - attempts, 0uLL);
		EXPECT_EQ(cacheSize(), 3uLL);
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

		// Note: Only the archive stage is built; blob and package come from the cache.
		EXPECT_EQ(insertAttempts() - attempts, 1uLL);
		EXPECT_EQ(packBefore, packageAddress());
		EXPECT_EQ(cacheSize(), 4uLL);
	}

	TEST_F(StackCacheIntegration, OverlappingQueriesShareStages) {
		auto storeA = archive(_guidA);
		auto storeB = archive(_guidB);
		ASSERT_NE(storeA, nullptr);
		ASSERT_NE(storeB, nullptr);

		bool innerRan = false;
		bool outerRan = false;
		{
			auto guardA = (_system.*fetchArchiveFn())(clone(storeA));
			EXPECT_TRUE(_system.query(clone(storeB), [&](_In_ ref<StorageReadonlyArchive>) { innerRan = true; }).has_value());
			EXPECT_TRUE(guardA([&](_In_ ref<StorageReadonlyArchive>) { outerRan = true; }).has_value());
		}

		EXPECT_TRUE(innerRan);
		EXPECT_TRUE(outerRan);
		// Note: Blob and package are shared; one stage per archive on top.
		EXPECT_EQ(cacheSize(), 4uLL);
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
}
