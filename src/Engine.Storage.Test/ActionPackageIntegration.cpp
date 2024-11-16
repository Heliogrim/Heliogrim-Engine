#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

#include <Engine.Common/Guid.hpp>
#include <Engine.Config/Config.hpp>
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
		const auto memUrl = MemoryUrl { clone(memGuid_) };
		constexpr auto requestSize = 4096uLL;
		constexpr auto properties = MemoryProperty::eUndefined;

		/**/

		auto descriptor = MemoryStorageDescriptor { clone(memUrl), requestSize, properties };
		auto memStorage = self_._registry.insert(::hg::move(descriptor));
		ASSERT_NE(memStorage, nullptr);
	}

	static auto preparePackageStorage(
		_Inout_ ref<MutationIntegrationFixture> self_,
		_In_ cref<Guid> memGuid_,
		_In_ cref<Guid> packGuid_
	) {
		const auto packUrl = PackageUrl { clone(packGuid_) };
		auto memStore = self_._registry.findStorageByUrl(MemoryUrl { clone(memGuid_) });

		/**/

		auto descriptor = PackageStorageDescriptor { clone(packUrl), ::hg::move(memStore) };
		auto packStorage = self_._registry.insert(::hg::move(descriptor));
		ASSERT_NE(packStorage, nullptr);
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

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			auto initialArchive = BufferArchive {};
			initialArchive.reserve(initial_archive_data.size());
			initialArchive.serializeBytes(
				const_cast<char*>(initial_archive_data.data()),
				initial_archive_data.size(),
				ArchiveStreamMode::eStore
			);

			_system.mutate(
				::hg::move(packStore).into<PackageStorage>(),
				[&initialArchive, &initialGuid = initial_archive_guid](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
					auto& linked = rwp_.getLinker().add({ .type = package::PackageArchiveType::eRaw, .guid = initialGuid });
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

		/* I : Validating insertion */

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			_system.query(
				::hg::move(packStore).into<PackageStorage>(),
				[
					&initialGuid = initial_archive_guid, initial_archive_size, initial_archive_data
				](_In_ cref<ReadonlyPackage> rp_) {

					ASSERT_GE(rp_.getHeader().packageSize, initial_archive_size);
					ASSERT_GE(rp_.getHeader().indexSize, sizeof(package::Indexed));

					EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

					/**/

					const auto& linker = rp_.getLinker();

					EXPECT_EQ(linker.count(), 1uLL);
					EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(package::Indexed));
					EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
					EXPECT_TRUE(linker.contains(static_cast<Guid>(initialGuid)));

					/**/

					auto linked = linker.get(initialGuid);
					ASSERT_TRUE(linked.has_value());

					EXPECT_EQ(linked->header.guid, initialGuid);
					EXPECT_EQ(linked->header.type, package::PackageArchiveType::eRaw);

					/**/

					constexpr auto contentOffset = sizeof(PackageHeader);
					auto archive = StorageReadonlyArchive {
						linker.getStorage(),
						static_cast<streamoff>(contentOffset + linked->data.offset),
						linked->data.size
					};
					ASSERT_EQ(archive.totalSize(), initial_archive_size);

					for (const auto& ch : initial_archive_data) {
						char ac {};
						archive >> ac;
						EXPECT_EQ(ch, ac);
					}
				}
			);
		}

		/* II : Replace initial data testing */

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			auto replaceArchive = BufferArchive {};
			replaceArchive.reserve(replace_archive_data.size());
			replaceArchive.serializeBytes(
				const_cast<char*>(replace_archive_data.data()),
				replace_archive_data.size(),
				ArchiveStreamMode::eStore
			);

			_system.mutate(
				::hg::move(packStore).into<PackageStorage>(),
				[&replaceArchive, &initialGuid = initial_archive_guid](_Inout_ ref<ReadWritePackage> rwp_) {

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

		/* II : Validating replacement */

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			_system.query(
				::hg::move(packStore).into<PackageStorage>(),
				[
					&initialGuid = initial_archive_guid, replace_archive_size, replace_archive_data
				](_In_ ref<const ReadonlyPackage> rp_) {

					ASSERT_GE(rp_.getHeader().packageSize, replace_archive_size);
					ASSERT_GE(rp_.getHeader().indexSize, sizeof(package::Indexed));

					EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

					/**/

					const auto& linker = rp_.getLinker();

					EXPECT_EQ(linker.count(), 1uLL);
					EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(package::Indexed));
					EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
					EXPECT_TRUE(linker.contains(static_cast<Guid>(initialGuid)));

					/**/

					auto linked = linker.get(initialGuid);
					ASSERT_TRUE(linked.has_value());

					EXPECT_EQ(linked->header.guid, initialGuid);
					EXPECT_EQ(linked->header.type, package::PackageArchiveType::eRaw);

					/**/

					constexpr auto contentOffset = sizeof(PackageHeader);
					auto archive = StorageReadonlyArchive {
						linker.getStorage(),
						static_cast<streamoff>(contentOffset + linked->data.offset),
						linked->data.size
					};
					ASSERT_EQ(archive.totalSize(), replace_archive_size);

					for (const auto& ch : replace_archive_data) {
						char ac {};
						archive >> ac;
						EXPECT_EQ(ch, ac);
					}
				}
			);
		}

		/* III : Removing replace data testing */

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			_system.mutate(
				::hg::move(packStore).into<PackageStorage>(),
				[&initialGuid = initial_archive_guid](_Inout_ ref<ReadWritePackage> rwp_) {

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

		/* III : Validating removal */

		{
			auto packStore = _registry.findStorageByUrl(PackageUrl { clone(packGuid) });
			ASSERT_NE(packStore, nullptr);

			_system.query(
				::hg::move(packStore).into<PackageStorage>(),
				[&initialGuid = initial_archive_guid](_In_ ref<const ReadonlyPackage> rp_) {

					ASSERT_EQ(rp_.getHeader().packageSize, 0uLL);
					ASSERT_GE(rp_.getHeader().indexSize, 0uLL);

					EXPECT_NE(reinterpret_cast<u64>(rp_.getFooter().crc32.data()), u64 {});

					/**/

					const auto& linker = rp_.getLinker();

					EXPECT_EQ(linker.count(), 0uLL);
					EXPECT_EQ(linker.computeArchiveSize(), 0uLL);
					EXPECT_EQ(linker.computeArchiveSize(), package::computeContentSize(rp_.getHeader()));
					EXPECT_FALSE(linker.contains(static_cast<Guid>(initialGuid)));
				}
			);
		}
	}
}
