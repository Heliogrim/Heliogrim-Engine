#include "StorageSystem.hpp"

#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Resource.Archive/StorageReadonlyArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Package/Attribute/PackageGuid.hpp>
#include <Engine.Storage.Package/Factory.hpp>
#include <Engine.Storage.Package/Pattern.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage.Registry/Options/ArchiveStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Url/Url.hpp>

#include "Details/StorageActionResolver.hpp"

using namespace hg::engine::storage;
using namespace hg;

/**/

constexpr static auto clone2IStorage = [](const auto& store_) {
	return clone(store_).template into<IStorage>();
};

constexpr static auto move2IStorage = [](auto&& store_) {
	return ::hg::move(store_).template into<IStorage>();
};

/**/

#pragma region Generic Storage Operations

bool StorageSystem::hasStorage(mref<ArchiveUrl> archiveUrl_) const noexcept {
	return _registry.hasStorage(::hg::move(archiveUrl_));
}

bool StorageSystem::hasStorage(mref<FileUrl> fileUrl_) const noexcept {
	return _registry.hasStorage(::hg::move(fileUrl_));
}

bool StorageSystem::hasStorage(mref<MemoryUrl> memoryUrl_) const noexcept {
	return _registry.hasStorage(::hg::move(memoryUrl_));
}

bool StorageSystem::hasStorage(mref<PackageUrl> packageUrl_) const noexcept {
	return _registry.hasStorage(::hg::move(packageUrl_));
}

Opt<Arci<IStorage>> StorageSystem::removeStorage(mref<FileUrl> fileUrl_) {

	auto baseStorage = _registry.findStorageByUrl(clone(fileUrl_));
	if (baseStorage == nullptr || not IsType<system::LocalFileStorage>(*baseStorage)) {
		return None;
	}

	/**/

	auto collector = Vector<Arci<IStorage>> {};

	_registry.findReferrerStorages(clone(ArchiveScheme), baseStorage, collector);
	_registry.findReferrerStorages(clone(PackageScheme), baseStorage, collector);

	/**/

	for (size_t idx = 0; idx < collector.size(); ++idx) {
		const auto& ref = collector[idx];
		_registry.findReferrerStorages(clone(ArchiveScheme), ref, collector);
		_registry.findReferrerStorages(clone(PackageScheme), ref, collector);
	}

	/**/

	// TODO: Throw, cause we are missing functionality to handle stacked storages yet.
	::hg::assertrt(collector.empty());

	auto reverse = std::views::reverse(collector);
	for (auto&& effected : reverse) {

		// TODO: [Finalize]
		// TODO: Commit all changes by effected storages
		// TODO: Invalidate all queries, mutation and correlated resources and corresponding access control objects

		// TODO: [Erase]
		// TODO: _registry.removeStorage(::hg::move(effected));
	}

	/**/

	return Some(_registry.removeStorageByUrl(::hg::move(fileUrl_)));
}

Opt<Arci<IStorage>> StorageSystem::removeStorage(mref<MemoryUrl> memoryUrl_) {

	auto baseStorage = _registry.findStorageByUrl(clone(memoryUrl_));
	if (baseStorage == nullptr || not IsType<system::MemoryStorage>(*baseStorage)) {
		return None;
	}

	/**/

	auto collector = Vector<Arci<IStorage>> {};

	_registry.findReferrerStorages(clone(ArchiveScheme), baseStorage, collector);
	_registry.findReferrerStorages(clone(PackageScheme), baseStorage, collector);

	/**/

	for (size_t idx = 0; idx < collector.size(); ++idx) {
		const auto& ref = collector[idx];
		_registry.findReferrerStorages(clone(ArchiveScheme), ref, collector);
		_registry.findReferrerStorages(clone(PackageScheme), ref, collector);
	}

	/**/

	// TODO: Throw, cause we are missing functionality to handle stacked storages yet.
	::hg::assertrt(collector.empty());

	auto reverse = std::views::reverse(collector);
	for (auto&& effected : reverse) {

		// TODO: [Finalize]
		// TODO: Commit all changes by effected storages
		// TODO: Invalidate all queries, mutation and correlated resources and corresponding access control objects

		// TODO: [Erase]
		// TODO: _registry.removeStorage(::hg::move(effected));
	}

	/**/

	return Some(_registry.removeStorageByUrl(::hg::move(memoryUrl_)));
}

#pragma endregion
#pragma region Package Storage Operations

Opt<Arci<system::PackageStorage>> StorageSystem::findPackage(mref<PackageUrl> packageUrl_) const noexcept {
	auto result = _registry.findStorageByUrl(::hg::move(packageUrl_));
	return result != nullptr && IsType<system::PackageStorage>(*result) ?
		Some(::hg::move(result).into<system::PackageStorage>()) :
		None;
}

bool StorageSystem::hasPackage(mref<PackageUrl> packageUrl_) const noexcept {
	return hasStorage(::hg::move(packageUrl_));
}

#pragma endregion
#pragma region Archive Storage Operations

Opt<Arci<system::ArchiveStorage>> StorageSystem::findArchive(mref<ArchiveUrl> archiveUrl_) const noexcept {
	auto result = _registry.findStorageByUrl(std::move(archiveUrl_));
	return result != nullptr && IsType<system::ArchiveStorage>(*result) ?
		Some(std::move(result).into<system::ArchiveStorage>()) :
		None;
}

bool StorageSystem::hasArchive(mref<ArchiveUrl> archiveUrl_) const noexcept {
	return hasStorage(::hg::move(archiveUrl_));
}

#pragma endregion
#pragma region Request Storage Operations

/**
 * system :: Local File Storage
 */

Result<Arci<system::LocalFileStorage>, request_file_error> StorageSystem::requestStorage(
	mref<RequestFileUrlOptions> options_
) {
	using result_type = Result<Arci<system::LocalFileStorage>, request_file_error>;

	auto known = _registry.findStorageByUrl(clone(options_.fileUrl));
	if (known != nullptr) {
		return result_type::expected(move(known).into<system::LocalFileStorage>());
	}

	// TODO: Create new storage object base on local filesystem node/file (hard link still missing)

	auto upsert = _registry.insert(FileStorageDescriptor { move(options_.fileUrl) });
	return result_type::expected(move(upsert).into<system::LocalFileStorage>());
}

/**
 * system :: Package Storage
 */

Result<Arci<system::PackageStorage>, request_package_error> StorageSystem::requestPackage(
	mref<RequestPackageStorageOptions> options_
) {
	using result_type = Result<Arci<system::PackageStorage>, request_package_error>;

	if (options_.storage.valueless_by_exception() || options_.storage.apply([](const auto& store_) { return store_ == nullptr; })) {
		return result_type::unexpected(
			std::invalid_argument { "Provided invalid storage object while requesting package storage." }
		);
	}

	/**/

	auto genericStore = ::hg::move(options_.storage).apply(move2IStorage);

	/**/

	auto collector = Vector<Arci<IStorage>> {};
	_registry.findReferrerStorages(
		clone(PackageScheme),
		genericStore,
		collector
	);

	::hg::assertrt(collector.size() < 2uLL);
	if (not collector.empty()) {
		::hg::assertrt(IsType<system::PackageStorage>(*collector.front()));
		return result_type::expected(move(collector.front()).into<system::PackageStorage>());
	}

	/**/

	// TODO: Create a new package storage based on the provided storage unit
	// TODO: Load data from underlying storage and recover the package meta data (e.g. package guid)

	auto recoverResult = query(
		clone(genericStore),
		[](_In_ ref<const resource::Blob> blob_) -> Opt<std::pair<resource::PackageGuid, Vector<ArchiveStorageDescriptor>>> {

			if (blob_.size() == 0LL) {
				return None;
			}

			// Error: This will break when using memory blob, as we have `-1` as a indeterminate size.

			::hg::assertrt(package::isPackageBlob(blob_));
			auto package = package::reconstruct_readonly_package(blob_);

			auto packageGuid = resource::PackageGuid::ntoh(package.getHeader().guid);
			::hg::assertd(packageGuid != resource::PackageGuid {});

			/**/

			auto packedArchives = Vector<ArchiveStorageDescriptor> {};
			for (const auto& linked : package.getLinker()) {
				packedArchives.emplace_back(ArchiveUrl { linked.first }, nullptr);
			}

			return Some(std::make_pair(::hg::move(packageGuid), ::hg::move(packedArchives)));
		}
	);

	/**/

	if (not recoverResult.has_value()) {
		return result_type::unexpected(
			std::invalid_argument { "Failed to determine whether to recover or renew the package based on the provided storage unit." }
		);
	}

	/**/

	if (*recoverResult == None) {

		auto guid = resource::PackageGuid {};
		GuidGenerate(guid);

		auto upsert = _registry.insert(
			PackageStorageDescriptor { PackageUrl { ::hg::move(guid) }, ::hg::move(genericStore) }
		);
		return result_type::expected(::hg::move(upsert).into<system::PackageStorage>());

	}

	/**/

	auto& recoverData = **recoverResult;
	auto upsert = _registry.insert(
		PackageStorageDescriptor {
			PackageUrl { ::hg::move(recoverData.first) }, ::hg::move(genericStore)
		}
	);

	for (auto&& archive : recoverData.second) {
		::hg::discard = _registry.insert(
			ArchiveStorageDescriptor {
				ArchiveUrl { archive.archiveGuid() },
				clone(upsert)
			}
		);
	}

	return result_type::expected(::hg::move(upsert).into<system::PackageStorage>());
}

Result<Arci<system::PackageStorage>, request_package_url_error> StorageSystem::requestPackage(
	mref<RequestPackageUrlOptions> options_
) {
	using result_type = Result<Arci<system::PackageStorage>, request_package_url_error>;

	auto fileResult = requestStorage({ .fileUrl = move(options_.fileUrl) });
	if (not fileResult.has_value()) {
		return result_type::unexpected(hg::move(fileResult).error());
	}

	// Question: How do we make efficient lookups for package storage units based on the backing storage? e.g. 'LocalFileStorage'

	// TODO: Replace with efficient solutions
	Vector<Arci<IStorage>> collector {};
	_registry.findReferrerStorages(clone(PackageScheme), clone(fileResult.value()).into<IStorage>(), collector);

	::hg::assertrt(collector.size() < 2uLL);
	if (not collector.empty()) {
		::hg::assertrt(IsType<system::PackageStorage>(*collector.front()));
		return result_type::expected(move(collector.front()).into<system::PackageStorage>());
	}

	/**/

	// TODO: Create new package storage based on file storage (hard link missing)
	resource::PackageGuid guid {};
	GuidGenerate(guid);

	auto upsert = _registry.insert(
		PackageStorageDescriptor {
			PackageUrl { move(guid) },
			hg::move(fileResult).value().into<IStorage>()
		}
	);
	return result_type::expected(move(upsert).into<system::PackageStorage>());
}

/**
 * system :: Archive Storage
 */

Result<Arci<system::ArchiveStorage>, request_archive_storage_located_error> StorageSystem::requestArchive(
	mref<ArchiveUrl> archiveUrl_,
	mref<RequestArchivePackageUrlOptions> options_
) {
	using result_type = Result<Arci<system::ArchiveStorage>, request_archive_storage_located_error>;

	auto known = findArchive(clone(archiveUrl_));
	Result<Arci<system::PackageStorage>, request_package_url_error> packageResult = requestPackage(
		move(options_.package)
	);

	if (not packageResult.has_value()) {
		return result_type::unexpected(
			request_archive_storage_located_error {
				hg::move(packageResult).error()
			}
		);
	}

	if (known != None) {
		return ((*known)->getBacking() == hg::move(packageResult).value().into<IStorage>()) ?
			result_type::expected(std::move(known).value()) :
			result_type::unexpected(std::runtime_error("Mismatch while comparing backing storage."));
	}

	/**/

	// TODO: Create new archive at package (hard link missing)
	auto upsert = _registry.insert(
		ArchiveStorageDescriptor { move(archiveUrl_), hg::move(packageResult).value().into<IStorage>() }
	);
	return result_type::expected(std::move(upsert).into<system::ArchiveStorage>());
}

#pragma endregion
#pragma region Fetch and Mutation Operations

IoQueryGuard<const engine::resource::Blob> StorageSystem::fetchQuery(
	mref<Arci<IStorage>> storage_
) const noexcept {

	::hg::assertd(IsType<system::MemoryStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<const resource::Blob>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>()
			}
		}
	};

	if (IsType<ISecondaryStorage>(*storage_)) {
		auto next = clone(static_cast<const ptr<ISecondaryStorage>>(storage_.get())->getBacking());
		while (next != nullptr) {

			const auto& fwd = *next;
			signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

			if (IsType<ISecondaryStorage>(fwd)) {
				next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
				continue;
			}

			break;
		}
	}

	/**/

	const auto factory = _resolver.resolveQuery(signature);

	// TODO: Replace with proper error handling
	::hg::assertrt(factory.has_value());

	/**/

	auto query = (*factory)();
	const auto prepareResult = query->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with proper error handling
	::hg::assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `query.cleanup(...)`?
	// Note: We may require actions to support an idempotent `cleanup` function.
	return IoQueryGuard<const resource::Blob> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		::hg::move(query)
	};
}

query_package_guard StorageSystem::fetchQuery(
	mref<Arci<system::PackageStorage>> storage_
) const noexcept {

	::hg::assertd(IsType<system::PackageStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<const resource::ReadonlyPackage>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::PackageStorage>()
			}
		}
	};

	auto next = clone(storage_->getBacking());
	while (next != nullptr) {

		const auto& fwd = *next;
		signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

		if (IsType<ISecondaryStorage>(fwd)) {
			next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
			continue;
		}

		break;
	}

	/**/

	const auto factory = _resolver.resolveQuery(signature);

	// TODO: Replace with explicit error handling
	::hg::assertrt(factory.has_value());

	/**/

	auto query = (*factory)();
	const auto prepareResult = query->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with explicit error handling
	::hg::assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `query.cleanup(...)`?
	return IoQueryGuard<const resource::ReadonlyPackage> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		hg::move(query)
	};
}

IoQueryGuard<engine::resource::StorageReadonlyArchive> StorageSystem::fetchQuery(
	mref<Arci<system::ArchiveStorage>> storage_
) const noexcept {

	assertd(IsType<system::ArchiveStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadonlyArchive>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>()
			}
		}
	};

	auto next = clone(storage_->getBacking());
	while (next != nullptr) {

		const auto& fwd = *next;
		signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

		if (IsType<ISecondaryStorage>(fwd)) {
			next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
			continue;
		}

		break;
	}

	/**/

	const auto factory = _resolver.resolveQuery(signature);

	// TODO: Replace with explicit error handling
	assertrt(factory.has_value());

	/**/

	auto query = (*factory)();
	const auto prepareResult = query->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with explicit error handling
	assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `query.cleanup(...)`?
	return IoQueryGuard<resource::StorageReadonlyArchive> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		hg::move(query)
	};
}

mutate_blob_guard StorageSystem::fetchMutation(
	mref<Arci<IStorage>> storage_
) const noexcept {

	::hg::assertd(IsType<system::MemoryStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<resource::Blob>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::MemoryStorage>()
			}
		}
	};

	if (IsType<ISecondaryStorage>(*storage_)) {
		auto next = clone(static_cast<const ptr<ISecondaryStorage>>(storage_.get())->getBacking());
		while (next != nullptr) {

			const auto& fwd = *next;
			signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

			if (IsType<ISecondaryStorage>(fwd)) {
				next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
				continue;
			}

			break;
		}
	}

	/**/

	const auto factory = _resolver.resolveMutation(signature);

	// TODO: Replace with proper error handling
	::hg::assertrt(factory.has_value());

	/**/

	auto mutation = (*factory)();
	const auto prepareResult = mutation->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with proper error handling
	::hg::assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `mutation.cleanup(...)`?
	// Note: We may require actions to support an idempotent `cleanup` function.
	return IoMutationGuard<resource::Blob> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		::hg::move(mutation)
	};
}

mutate_package_guard StorageSystem::fetchMutation(
	mref<Arci<system::PackageStorage>> storage_
) const noexcept {

	::hg::assertd(IsType<system::PackageStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<resource::ReadWritePackage>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::PackageStorage>()
			}
		}
	};

	auto next = clone(storage_->getBacking());
	while (next != nullptr) {

		const auto& fwd = *next;
		signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

		if (IsType<ISecondaryStorage>(fwd)) {
			next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
			continue;
		}

		break;
	}

	/**/

	const auto factory = _resolver.resolveMutation(signature);

	// TODO: Replace with explicit error handling
	::hg::assertrt(factory.has_value());

	/**/

	auto mutation = (*factory)();
	const auto prepareResult = mutation->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with explicit error handling
	::hg::assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `mutation.cleanup(...)`?
	return IoMutationGuard<resource::ReadWritePackage> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		hg::move(mutation)
	};
}

IoMutationGuard<engine::resource::StorageReadWriteArchive> StorageSystem::fetchMutation(
	mref<Arci<system::ArchiveStorage>> storage_
) const noexcept {

	assertd(IsType<system::ArchiveStorage>(*storage_));

	/**/

	auto signature = ActionTypeInfo {
		.targetResourceInfo = refl::FullTypeInfo::from<resource::StorageReadWriteArchive>(),
		.stageTypeInfos = {
			StageTypeInfo {
				.storage = refl::PartialTypeInfo::from<system::ArchiveStorage>()
			}
		}
	};

	auto next = clone(storage_->getBacking());
	while (next != nullptr) {

		const auto& fwd = *next;
		signature.stageTypeInfos.emplace_back(StageTypeInfo { refl::PartialTypeInfo { fwd.getMetaClass() } });

		if (IsType<ISecondaryStorage>(fwd)) {
			next = static_cast<cref<ISecondaryStorage>>(fwd).getBacking();
			continue;
		}

		break;
	}

	/**/

	const auto factory = _resolver.resolveMutation(signature);

	// TODO: Replace with explicit error handling
	assertrt(factory.has_value());

	/**/

	auto mutation = (*factory)();
	const auto prepareResult = mutation->prepare(move(storage_).into<IStorage>());

	// TODO: Replace with explicit error handling
	assertrt(prepareResult.has_value());

	/**/

	// Problem:
	// Question: Where do we call `mutation.cleanup(...)`?
	return IoMutationGuard<resource::StorageReadWriteArchive> {
		static_cast<ref<ExtendedStorageSystem>>(const_cast<ref<StorageSystem>>(*this)),
		hg::move(mutation)
	};
}

#pragma endregion
#pragma region Support Operations

Result<Arci<system::ArchiveStorage>, mutate_package_error> StorageSystem::addArchiveToPackage(
	mref<Arci<system::PackageStorage>> package_,
	mref<AddArchiveToPackageOptions> options_
) {

	if (package_ == nullptr) {
		return Unexpected<mutate_package_error> { std::invalid_argument { "Tried to add an archive to a `null` package." } };
	}

	/**/

	auto result = mutate(
		clone(package_),
		[_opt = &options_](_Inout_ ref<resource::ReadWritePackage> rwp_) {

			auto guid = resource::ArchiveGuid::random();

			auto& linker = rwp_.getLinker();
			auto linked = linker.add(resource::package::PackageArchiveHeader { .type = _opt->archive.type(), .guid = ::hg::move(guid) });
			linked.changes.emplace_back(
				resource::package::ArchiveDeltaAdd {
					streamoff {}, _opt->archive.totalSize(), _opt->archive
				}
			);

			/**/

			// TODO: Commit changes of linker to underlying binary io interface
			::hg::todo_panic();
		}
	);

	return Expected { Arci<system::ArchiveStorage> {} };
}

Result<Vector<Arci<system::ArchiveStorage>>, query_blob_error> StorageSystem::enumArchivesFromPackage(
	mref<Arci<system::PackageStorage>> package_
) {

	if (package_ == nullptr) {
		return Unexpected<query_blob_error> { std::invalid_argument { "Tried to enumerate archives of `null` package." } };
	}

	/**/

	auto collector = Vector<Arci<IStorage>> {};
	_registry.findReferrerStorages(clone(ArchiveScheme), ::hg::move(package_).into<IStorage>(), collector);

	if (collector.empty()) {
		return Expected { Vector<Arci<system::ArchiveStorage>> {} };
	}

	/**/

	// @formatter:off
	[[maybe_unused]] const auto erased = std::erase_if(collector, [](cref<Arci<IStorage>> storage_) {
		return not IsType<system::ArchiveStorage>(*storage_);
	});
	::hg::assertd(erased == 0uLL);
	// @formatter:on

	auto enumSet = Vector<Arci<system::ArchiveStorage>> {};
	enumSet.reserve(collector.size());

	for (auto&& storage : collector) {
		enumSet.emplace_back(::hg::move(storage).into<system::ArchiveStorage>());
	}

	return Expected { ::hg::move(enumSet) };
}

#pragma endregion
