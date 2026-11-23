#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <Engine.Storage.System/Options/PackageRequestOptions.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

import Engine.Common;
import Engine.Config;
import Engine.Resource.Archive;
import Engine.Resource.Blob;
import Engine.Resource.Package;
import Engine.Storage.Action;
import Engine.Storage.Registry;
import Engine.Storage.Package;

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
					::hg::engine::resource::package::ArchiveDeltaAdd {
						.where = streamoff {},
						.sizedData = initialArchive
					}
				);
				return true;
			}
		);
	}

	/**
	 * The sealed footer checksum has to equal the checksum recomputed over the stored bytes
	 */
	static void expectSealedChecksum(const StorageSystem& system_, mref<Arci<IStorage>> packStore_) {
		auto backing = static_cast<ref<const ISecondaryStorage>>(*packStore_).getBacking();
		system_.query(
			::hg::move(backing),
			[](_In_ ref<const Blob> blob_) {
				auto opened = ::hg::engine::storage::package::reconstruct_readonly_package(blob_);
				ASSERT_TRUE(opened.has_value());

				const auto& rp = opened.value();
				const auto covered = static_cast<streamsize>(sizeof(PackageHeader) + rp.getHeader().packageSize);
				EXPECT_EQ(
					::hg::engine::storage::package::readChecksum(rp.getFooter()),
					::hg::engine::storage::package::computeChecksum(blob_, covered).value()
				);
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
		expectSealedChecksum(system_, clone(packStore));

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[
				&initialGuid = initialArchiveGuid_, initialArchiveSize_, initialArchiveData_
			](_In_ cref<ReadonlyPackage> rp_) {

				ASSERT_GE(rp_.getHeader().packageSize, initialArchiveSize_);
				ASSERT_GE(rp_.getHeader().indexSize, sizeof(::hg::engine::resource::package::Indexed));


				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 1uLL);
				EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(::hg::engine::resource::package::Indexed));
				EXPECT_EQ(linker.computeArchiveSize(), ::hg::engine::resource::package::computeContentSize(rp_.getHeader()));
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
					::hg::engine::resource::package::ArchiveDeltaReplace {
						.where = streamoff {},
						.byteCount = static_cast<streamsize>(replaceArchive.size()),
						.withSizedData = replaceArchive
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
		expectSealedChecksum(system_, clone(packStore));

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[
				&initialGuid = initialArchiveGuid_, replaceArchiveSize_, replaceArchiveData_
			](_In_ ref<const ReadonlyPackage> rp_) {

				ASSERT_GE(rp_.getHeader().packageSize, replaceArchiveSize_);
				ASSERT_GE(rp_.getHeader().indexSize, sizeof(::hg::engine::resource::package::Indexed));


				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 1uLL);
				EXPECT_EQ(linker.computeArchiveSize(), rp_.getHeader().packageSize - sizeof(::hg::engine::resource::package::Indexed));
				EXPECT_EQ(linker.computeArchiveSize(), ::hg::engine::resource::package::computeContentSize(rp_.getHeader()));
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
					::hg::engine::resource::package::ArchiveDeltaDrop {
						.where = streamoff {},
						.byteCount = linked->data.size
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
		expectSealedChecksum(system_, clone(packStore));

		system_.query(
			::hg::move(packStore).into<PackageStorage>(),
			[&initialGuid = initialArchiveGuid_](_In_ ref<const ReadonlyPackage> rp_) {

				ASSERT_EQ(rp_.getHeader().packageSize, 0uLL);
				ASSERT_GE(rp_.getHeader().indexSize, 0uLL);


				/**/

				const auto& linker = rp_.getLinker();

				EXPECT_EQ(linker.count(), 0uLL);
				EXPECT_EQ(linker.computeArchiveSize(), 0uLL);
				EXPECT_EQ(linker.computeArchiveSize(), ::hg::engine::resource::package::computeContentSize(rp_.getHeader()));
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

	[[nodiscard]] static Arci<IStorage> makeMemoryStorage(ref<MutationIntegrationFixture> self_) {
		auto guid = Guid {};
		GuidGenerate(guid);
		auto storage = self_._registry.insert(MemoryStorageDescriptor { MemoryUrl { ::hg::move(guid) }, 512uLL, MemoryProperty::eUndefined });
		return storage;
	}

	TEST_F(MutationIntegrationFixture, RequestPackageRenewsEmptyStorage) {
		auto memory = makeMemoryStorage(*this);
		const auto package = _system.requestPackage(RequestPackageStorageOptions { .storage = clone(memory).into<MemoryStorage>() });
		ASSERT_TRUE(package.has_value());
		EXPECT_NE(package.value(), nullptr);
	}

	TEST_F(MutationIntegrationFixture, RequestPackageRecoversSealedPackage) {
		auto memory = makeMemoryStorage(*this);
		const auto archiveGuid = ArchiveGuid::random();
		const auto written = _system.mutate(
			clone(memory),
			[&archiveGuid](_In_ ref<Blob> blob_) {
				auto package = ::hg::engine::storage::package::make_read_write_package(blob_);
				auto data = BufferArchive {};
				auto payload = StringView { "recover me" };
				data.serializeBytes(const_cast<char*>(payload.data()), payload.size(), ArchiveStreamMode::eStore);
				data.setType(ArchiveType::eRaw);
				auto& linked = package.getLinker().add({ .type = ArchiveType::eUndefined, .guid = archiveGuid });
				linked.changes.emplace_back(::hg::engine::resource::package::ArchiveDeltaAdd { .where = streamoff {}, .sizedData = data });
				return ::hg::engine::storage::package::commit_package_changes(package).has_value();
			}
		);
		ASSERT_TRUE(written.has_value());
		ASSERT_TRUE(written.value());

		const auto package = _system.requestPackage(RequestPackageStorageOptions { .storage = clone(memory).into<MemoryStorage>() });
		ASSERT_TRUE(package.has_value());
		EXPECT_NE(_registry.findStorageByUrl(ArchiveUrl { archiveGuid }), nullptr);

		auto referrers = Vector<Arci<IStorage>> {};
		_registry.findReferrerStorages(clone(PackageScheme), memory, referrers);
		EXPECT_EQ(referrers.size(), 1uLL);
	}

	TEST_F(MutationIntegrationFixture, RequestPackageRejectsCorruptBlob) {
		auto memory = makeMemoryStorage(*this);
		const auto written = _system.mutate(
			clone(memory),
			[](_In_ ref<Blob> blob_) {
				Array<_::byte, 256> garbage {};
				garbage.fill(_::byte { 0xAB });
				blob_.write(streamoff {}, std::span<_::byte> { garbage });
				return blob_.size();
			}
		);
		ASSERT_TRUE(written.has_value());
		ASSERT_GT(written.value(), 0LL);

		const auto package = _system.requestPackage(RequestPackageStorageOptions { .storage = clone(memory).into<MemoryStorage>() });
		ASSERT_FALSE(package.has_value());
		ASSERT_TRUE(package.error().is<std::invalid_argument>());
		EXPECT_NE(String { package.error().as<std::invalid_argument>().what() }.find("package structure is invalid"), String::npos);

		// Nothing half-built: no package storage refers to the rejected backing.
		auto referrers = Vector<Arci<IStorage>> {};
		_registry.findReferrerStorages(clone(PackageScheme), memory, referrers);
		EXPECT_TRUE(referrers.empty());

		// A package storage forced onto the corrupt backing refuses both access paths.
		auto packGuid = Guid {};
		GuidGenerate(packGuid);
		auto packStore = _registry.insert(PackageStorageDescriptor { PackageUrl { clone(packGuid) }, clone(memory) });
		ASSERT_NE(packStore, nullptr);

		bool reached = false;
		const auto queried = _system.query(
			clone(packStore).into<PackageStorage>(),
			[&reached](_In_ ref<const ReadonlyPackage>) {
				reached = true;
			}
		);
		EXPECT_FALSE(queried.has_value());

		const auto mutated = _system.mutate(
			clone(packStore).into<PackageStorage>(),
			[&reached](_Inout_ ref<ReadWritePackage>) -> bool {
				reached = true;
				return true;
			}
		);
		EXPECT_FALSE(mutated.has_value());
		EXPECT_FALSE(reached);

		// The rejected backing keeps its bytes.
		const auto first = _system.query(
			clone(memory),
			[](_In_ ref<const Blob> blob_) {
				auto byte = _::byte {};
				std::ignore = blob_.read(streamoff {}, std::span<_::byte> { &byte, 1uLL });
				return byte;
			}
		);
		ASSERT_TRUE(first.has_value());
		EXPECT_EQ(first.value(), _::byte { 0xAB });
	}
}
