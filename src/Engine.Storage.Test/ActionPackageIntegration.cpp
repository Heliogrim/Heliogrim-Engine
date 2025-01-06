#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <Engine.Common/Guid.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Resource.Archive/ArchiveType.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>
#include <Engine.Resource.Package/Linker/Indexed.hpp>
#include <Engine.Storage.Action/Context.hpp>
#include <Engine.Storage.Action/Access/ExclusiveIoResource.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/MemoryStorageOption.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.Registry/Url/MemoryUrl.hpp>
#include <Engine.Storage.Registry/Url/Url.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

#include "Utils.hpp"
#include "Attribute/PackageGuid.hpp"

/**/

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg;

/**/

namespace StorageModule {
	static auto preparePrimaryStorage(
		_Inout_ ref<MutationIntegrationFixture> self_,
		_In_ cref<Guid> memGuid_
	) {
		auto memUrl = MemoryUrl { clone(memGuid_) };
		constexpr auto requestSize = 4096uLL;
		constexpr auto properties = MemoryProperty::eUndefined;

		/**/

		auto descriptor = MemoryStorageDescriptor { ::hg::move(memUrl), requestSize, properties };
		auto memStorage = self_._registry.insert(::hg::move(descriptor));
		ASSERT_NE(memStorage, nullptr);
	}

	static auto preparePackageStorage(
		_Inout_ ref<MutationIntegrationFixture> self_,
		_In_ cref<Guid> memGuid_,
		_In_ cref<Guid> packGuid_
	) {
		auto packUrl = PackageUrl { clone(packGuid_) };
		auto memStore = self_._registry.findStorageByUrl(MemoryUrl { clone(memGuid_) });

		/**/

		auto descriptor = PackageStorageDescriptor { ::hg::move(packUrl), ::hg::move(memStore) };
		auto packStorage = self_._registry.insert(::hg::move(descriptor));
		ASSERT_NE(packStorage, nullptr);
	}

	static auto insertionTest(
		IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const ArchiveGuid& initialArchiveGuid_,
		auto& initialArchiveData_,
		const StorageSystem& system_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		auto initialArchive = BufferArchive {};
		initialArchive.reserve(initialArchiveData_.size());
		initialArchive.serializeBytes(
			const_cast<char*>(initialArchiveData_.data()),
			initialArchiveData_.size(),
			ArchiveStreamMode::eStore
		);
		initialArchive.setType(ArchiveType::eRaw);

		system_.mutate(
			::hg::move(packStore).into<PackageStorage>(),
			[&initialArchive, &initialGuid = initialArchiveGuid_](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
				auto& linked = rwp_.getLinker().add({ .type = ArchiveType::eUndefined, .guid = initialGuid });
				linked.changes.emplace_back(
					package::ArchiveDeltaAdd {
						.where = streamoff {},
						.size = static_cast<streamsize>(initialArchive.size()),
						.data = initialArchive
					}
				);
				return true;
			}
		);
	}

	static auto validateInsertionTest(
		IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const StorageSystem& system_,
		const ArchiveGuid& initialArchiveGuid_,
		const auto& initialArchiveSize_,
		const auto& initialArchiveData_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[
				&initialGuid = initialArchiveGuid_, initialArchiveSize_, initialArchiveData_
			](_In_ cref<ReadonlyPackage> rp_) {

				ASSERT_GE(rp_.getHeader().packageSize, initialArchiveSize_);
				ASSERT_GE(rp_.getHeader().indexSize, sizeof(package::Indexed));

				EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 1uLL);
				EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(package::Indexed));
				EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
				EXPECT_TRUE(linker.contains(initialGuid));

				/**/

				auto linked = linker.get(initialGuid);
				ASSERT_TRUE(linked.has_value());

				EXPECT_EQ(linked->header.guid, initialGuid);
				EXPECT_EQ(linked->header.type, ArchiveType::eRaw);

				/**/

				constexpr auto contentOffset = sizeof(PackageHeader);
				auto archive = StorageReadonlyArchive {
					linker.getStorage(),
					static_cast<streamoff>(contentOffset + linked->data.offset),
					linked->data.size
				};
				ASSERT_EQ(archive.totalSize(), initialArchiveSize_);

				for (const auto& ch : initialArchiveData_) {
					char ac {};
					archive >> ac;
					EXPECT_EQ(ch, ac);
				}
			}
		);
	}

	static auto replaceTest(
		IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const auto& replaceArchiveData_,
		const StorageSystem& system_,
		const ArchiveGuid& initialArchiveGuid_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		auto replaceArchive = BufferArchive {};
		replaceArchive.reserve(replaceArchiveData_.size());
		replaceArchive.serializeBytes(
			const_cast<char*>(replaceArchiveData_.data()),
			replaceArchiveData_.size(),
			ArchiveStreamMode::eStore
		);
		replaceArchive.setType(ArchiveType::eSerializedStructure);

		system_.mutate(
			::hg::move(packStore).into<PackageStorage>(),
			[&replaceArchive, &initialGuid = initialArchiveGuid_](_Inout_ ref<ReadWritePackage> rwp_) {

				auto linked = rwp_.getLinker().get(initialGuid);
				ASSERT_TRUE(linked.has_value());

				linked->changes.emplace_back(
					package::ArchiveDeltaReplace {
						.where = streamoff {},
						.size = static_cast<streamsize>(replaceArchive.size()),
						.data = replaceArchive
					}
				);
			}
		);
	}

	static auto validateReplaceTest(
		const IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const StorageSystem& system_,
		const ArchiveGuid& initialArchiveGuid_,
		const auto& replaceArchiveSize_,
		const auto& replaceArchiveData_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[
				&initialGuid = initialArchiveGuid_, replaceArchiveSize_, replaceArchiveData_
			](_In_ ref<const ReadonlyPackage> rp_) {

				ASSERT_GE(rp_.getHeader().packageSize, replaceArchiveSize_);
				ASSERT_GE(rp_.getHeader().indexSize, sizeof(package::Indexed));

				EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 1uLL);
				EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(package::Indexed));
				EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
				EXPECT_TRUE(linker.contains(initialGuid));

				/**/

				auto linked = linker.get(initialGuid);
				ASSERT_TRUE(linked.has_value());

				EXPECT_EQ(linked->header.guid, initialGuid);
				EXPECT_EQ(linked->header.type, ArchiveType::eSerializedStructure);

				/**/

				constexpr auto contentOffset = sizeof(PackageHeader);
				auto archive = StorageReadonlyArchive {
					linker.getStorage(),
					static_cast<streamoff>(contentOffset + linked->data.offset),
					linked->data.size
				};
				ASSERT_EQ(archive.totalSize(), replaceArchiveSize_);

				for (const auto& ch : replaceArchiveData_) {
					char ac {};
					archive >> ac;
					EXPECT_EQ(ch, ac);
				}
			}
		);
	}

	static auto removeTest(
		const IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const StorageSystem& system_,
		const auto& initialArchiveGuid_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		system_.mutate(
			::hg::move(packStore).into<PackageStorage>(),
			[&initialGuid = initialArchiveGuid_](_Inout_ ref<ReadWritePackage> rwp_) {

				auto linked = rwp_.getLinker().get(initialGuid);
				ASSERT_TRUE(linked.has_value());

				linked->changes.emplace_back(
					package::ArchiveDeltaDrop {
						.where = streamoff {},
						.size = linked->data.size
					}
				);
			}
		);
	}

	static auto validateRemoveTest(
		const IStorageRegistry& registry_,
		const PackageGuid& packGuid_,
		const StorageSystem& system_,
		const auto& initialArchiveGuid_
	) {
		auto packStore = registry_.findStorageByUrl(PackageUrl { clone(packGuid_) });
		ASSERT_NE(packStore, nullptr);

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[&initialGuid = initialArchiveGuid_](_In_ ref<const ReadonlyPackage> rp_) {

				ASSERT_EQ(rp_.getHeader().packageSize, 0uLL);
				ASSERT_GE(rp_.getHeader().indexSize, 0uLL);

				EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 0uLL);
				EXPECT_EQ(linker.computeArchiveSize(), 0uLL);
				EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
				EXPECT_FALSE(linker.contains(initialGuid));
			}
		);
	}

	TEST_F(MutationIntegrationFixture, MultiStepPackageDirectCycle) {

		auto memGuid = Guid {};
		GuidGenerate(memGuid);

		auto packGuid = Guid {};
		GuidGenerate(packGuid);

		const auto initial_archive_guid = ArchiveGuid::random();
		constexpr auto initial_archive_data = StringView { "Initial Archive Data"sv };
		constexpr auto initial_archive_size = initial_archive_data.size();

		constexpr auto replace_archive_data = StringView { "Replace Archive Data"sv };
		constexpr auto replace_archive_size = replace_archive_data.size();

		/**/

		preparePrimaryStorage(*this, memGuid);
		preparePackageStorage(*this, memGuid, packGuid);

		/* I : Insertion testing */

		insertionTest(_registry, packGuid, initial_archive_guid, initial_archive_data, _system);

		/* I : Validating insertion */

		validateInsertionTest(_registry, packGuid, _system, initial_archive_guid, initial_archive_size, initial_archive_data);

		/* II : Replace initial data testing */

		replaceTest(_registry, packGuid, replace_archive_data, _system, initial_archive_guid);

		/* II : Validating replacement */

		validateReplaceTest(_registry, packGuid, _system, initial_archive_guid, replace_archive_size, replace_archive_data);

		/* III : Removing replace data testing */

		removeTest(_registry, packGuid, _system, initial_archive_guid);

		/* III : Validating removal */

		validateRemoveTest(_registry, packGuid, _system, initial_archive_guid);
	}
}
