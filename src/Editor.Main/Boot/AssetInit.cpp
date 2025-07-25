#include "AssetInit.hpp"

/**/
#include "Engine.Assets.Type/Geometry/StaticGeometry.hpp"
#include "Engine.Assets.Type/Texture/FontAsset.hpp"
#include "Engine.Assets.Type/Texture/ImageAsset.hpp"
#include "Engine.Assets.Type/Texture/TextureAsset.hpp"
#include "Editor.Action/Action/Action.hpp"
/**/

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Async/Await.hpp>

#include "Editor.Assets.Default/DefaultAssetInit.hpp"
#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/AutoImportAction.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets/Utils.hpp"
#include "Engine.Config/Config.hpp"
#include "Engine.Config/Enums.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.Reflect/IsType.hpp"
#include "Engine.Resource.Archive/BufferArchive.hpp"
#include "Engine.Resource.Archive/StorageReadonlyArchive.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Resource.Package/Attribute/MagicBytes.hpp"
#include "Engine.Resource.Package/Attribute/PackageGuid.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization/Scheme/DefaultScheme.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"
#include "Engine.Storage.Package/Pattern.hpp"
#include "Engine.Storage.Registry/IStorage.hpp"
#include "Engine.Storage.Registry/IStorageRegistry.hpp"
#include "Engine.Storage/StorageModule.hpp"
#include "Engine.Storage.Registry/Options/FileStorageDescriptor.hpp"
#include "Engine.Storage.Registry/Options/PackageStorageDescriptor.hpp"
#include "Engine.Storage.Registry/Options/StorageDescriptor.hpp"
#include "Engine.Storage.Registry/Storage/LocalFileStorage.hpp"
#include "Engine.Storage.Registry/Storage/PackageStorage.hpp"
#include "Engine.Storage.System/StorageSystem.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

using path = std::filesystem::path;

namespace hg::engine::serialization {
	class RecordScopedSlot;
}

struct Indexed {
	std::filesystem::path primary;
	size_t primaryCount;

	std::filesystem::path meta;
	Opt<concurrent::future<void>> scheduled;
};

struct RetryContext {
	assets::AssetName baseName;
	size_t retry;
};

/**/

// @formatter:off
static void package_discover_assets(mref<Arci<storage::system::PackageStorage>> packStore_);
static bool is_archived_asset(mref<serialization::RecordScopedSlot> record_);
static bool try_load_archived_asset(ref<const storage::ArchiveUrl> assetStorageUrl_, mref<serialization::RecordScopedSlot> record_);
static bool auto_import(ref<Indexed> indexed_);
static bool try_recover_vfs(ref<assets::Asset> asset_, ref<const storage::ArchiveUrl> archiveUrl_);
static Opt<::hg::engine::fs::Path> find_best_match_relative(ref<const ::hg::engine::fs::Path> lfsStoragePath_);
// @formatter:on

/**/

#pragma region File System Loading

#if ENV_WIN
constexpr static auto prefix_extension = L'.';
constexpr static auto packed_extension = std::wstring_view { L".imp" };
#else
constexpr static auto prefix_extension = '.';
constexpr static auto packed_extension = std::string_view { R"(.imp)" };
#endif

// @formatter:off
[[nodiscard]] static bool matchingMagicBytes(cref<path> lfs_);
[[nodiscard]] static bool isPackedFile(cref<path> lfs_);
[[nodiscard]] static bool convergentBundleName(cref<path> lfsBundle_);
// @formatter:on

// @formatter:off
[[nodiscard]] static bool isPackedAsset(cref<path> lfs_);
[[nodiscard]] static bool isMetaAsset(cref<path> lfs_);
[[nodiscard]] static bool isOpaqueAsset(cref<path> lfs_);
[[nodiscard]] static bool isBundleAsset(cref<path> lfs_);
// @formatter:on

// @formatter:off
[[nodiscard]] static Opt<path> getMetaAssetFor(cref<path> lfsBundleOrOpaque_);
[[nodiscard]] static Opt<path> getBundleOrOpaqueFor(cref<path> lfsMeta_);
// @formatter:on

/**/

bool matchingMagicBytes(cref<path> lfs_) {
	auto fstream = std::fstream { lfs_, std::ios::in | std::ios::binary };
	Array<_::byte, resource::PackageMagicBytes.size()> buffer {};
	fstream.read(std::bit_cast<char*>(buffer.data()), buffer.size());
	return std::memcmp(buffer.data(), resource::PackageMagicBytes.data(), resource::PackageMagicBytes.size()) == 0;
}

bool isPackedFile(cref<path> lfs_) {
	return lfs_.native().ends_with(packed_extension) && matchingMagicBytes(lfs_);
}

bool convergentBundleName(cref<path> lfsBundle_) {

	const auto range = std::filesystem::directory_iterator {
		lfsBundle_, std::filesystem::directory_options::follow_directory_symlink
	};

	const auto candidate = clone(lfsBundle_).append(lfsBundle_.filename().native()).native();
	auto accounted = 0uLL;

	for (auto& effector : range) {

		if (effector.is_directory()) {
			// Note: Fail on nested structure.
			return false;
		}

		if (not isOpaqueAsset(effector)) {
			// Note: We expect to only discover opaque assets within a bundle.
			return false;
		}

		const auto& epn = effector.path().native();
		if (not epn.starts_with(candidate)) {
			return false;
		}

		if (epn.size() > candidate.size() && epn.at(candidate.size()) != prefix_extension) {
			// Note: We expect all opaque assets to shared the bundle name, but will allow multiple extension suffixes.
			return false;
		}

		++accounted;
	}

	// Note: We expect a bundle to consist of more than one opaque asset.
	return accounted > 1uLL;
}

/**/

bool isPackedAsset(cref<path> lfs_) {
	return std::filesystem::is_regular_file(lfs_) && isPackedFile(lfs_) && not getBundleOrOpaqueFor(lfs_).has_value();
}

bool isMetaAsset(cref<path> lfs_) {
	return std::filesystem::is_regular_file(lfs_) && isPackedFile(lfs_) && getBundleOrOpaqueFor(lfs_).has_value();
}

bool isOpaqueAsset(cref<path> lfs_) {
	return std::filesystem::is_regular_file(lfs_) && not isPackedFile(lfs_);
}

bool isBundleAsset(cref<path> lfs_) {
	return std::filesystem::is_directory(lfs_) && (getMetaAssetFor(lfs_).has_value() || convergentBundleName(lfs_));
}

/**/

Opt<path> getMetaAssetFor(cref<path> lfsBundleOrOpaque_) {
	auto expanded = clone(lfsBundleOrOpaque_) += packed_extension;
	return std::filesystem::exists(expanded) ? Some(std::move(expanded)) : None;
}

Opt<path> getBundleOrOpaqueFor(cref<path> lfsMeta_) {
	auto stripped = path { lfsMeta_.native().substr(0uLL, lfsMeta_.native().size() - packed_extension.size()) };
	return (isOpaqueAsset(stripped) || isBundleAsset(stripped)) ? Some(std::move(stripped)) : None;
}

/**/

using LoadAwait = concurrent::future<void>;

// @formatter:off
[[nodiscard]] static Opt<Indexed> buildIndexedFrom(cref<path> lfs_);
[[nodiscard]] static Opt<LoadAwait> enqueueIndexedLoad(cref<Indexed> indexed_);
// @formatter:on

// @formatter:off
static ref<Indexed> upsertIndexed(ref<Vector<Indexed>> set_, mref<Indexed> indexed_);
// @formatter:on

/**/

Opt<Indexed> buildIndexedFrom(cref<path> lfs_) {

	if (isOpaqueAsset(lfs_)) {
		return Some(Indexed { .primary = lfs_, .primaryCount = 1uLL, .meta = {}, .scheduled = None });
	}

	if (isBundleAsset(lfs_)) {
		return Some(Indexed { .primary = lfs_, .primaryCount = 1uLL, .meta = {}, .scheduled = None });
	}

	if (isPackedAsset(lfs_)) {
		return Some(Indexed { .primary = lfs_, .primaryCount = 1uLL, .meta = lfs_, .scheduled = None });
	}

	if (isMetaAsset(lfs_)) {
		return Some(Indexed { .primary = {}, .primaryCount = 0uLL, .meta = lfs_, .scheduled = None });
	}

	return None;
}

Opt<LoadAwait> enqueueIndexedLoad(cref<Indexed> indexed_) {

	const auto engine = Engine::getEngine();
	const auto storeSys = engine->getStorage()->getSystem();

	/**/

	auto maybeMetaFile = storeSys->requestStorage(
		{ .fileUrl = { clone(storage::FileProjectScheme), engine::fs::Path { indexed_.meta } } }
	);

	if (not maybeMetaFile.has_value()) {
		IM_CORE_ERRORF("Failed to request indexed meta file `{}`.", indexed_.meta.string());
		return None;
	}

	/**/

	const auto blobIsPack = storeSys->query(
		clone(maybeMetaFile.value()).into<storage::IStorage>(),
		[](_In_ ref<const resource::Blob> blob_) {
			return storage::package::isPackageBlob(blob_);
		}
	);

	if (not blobIsPack.has_value() || not blobIsPack.value()) {
		IM_CORE_ERRORF(
			"Encountered indexed meta file `{}` which is not identifiable as package upon inspection.",
			indexed_.meta.string()
		);
		return None;
	}

	/**/

	auto maybePackStore = storeSys->requestPackage({ .storage = ::hg::move(maybeMetaFile).value() });

	if (not maybePackStore.has_value()) {
		IM_CORE_ERRORF("Failed to request package storage based on index meta file `{}`.", indexed_.meta.string());
		return None;
	}

	/**/

	// @formatter:off
	return Some(scheduler::async<void>([storePack = std::move(maybePackStore).value()]() mutable {
		package_discover_assets(std::move(storePack));
	}));
	// @formatter:on
}

/**/

ref<Indexed> upsertIndexed(ref<Vector<Indexed>> set_, mref<Indexed> indexed_) {

	const auto primaryKey = indexed_.primary.empty() ?
		getBundleOrOpaqueFor(indexed_.meta) :
		Some(clone(indexed_.primary));
	const auto metaKey = indexed_.meta.empty() ?
		getMetaAssetFor(indexed_.primary) :
		Some(clone(indexed_.meta));

	auto existsByPrimary = primaryKey.has_value() ?
		std::ranges::find(
			set_,
			primaryKey.value(),
			[](const auto& stored_) { return stored_.primary; }
		) :
		std::ranges::end(set_);

	auto existsByMeta = metaKey.has_value() ?
		std::ranges::find(
			set_,
			metaKey.value(),
			[](const auto& stored_) { return stored_.meta; }
		) :
		std::ranges::end(set_);

	/**/

	if (not indexed_.primary.empty() && not indexed_.meta.empty()) {
		::hg::assertrt(existsByPrimary == set_.end());
		::hg::assertrt(existsByMeta == set_.end());

		return set_.emplace_back(std::move(indexed_));
	}

	if (not indexed_.primary.empty() && indexed_.meta.empty()) {
		::hg::assertrt(existsByPrimary == set_.end());

		if (existsByMeta != set_.end()) {
			existsByMeta->primary = indexed_.primary;
			existsByMeta->primaryCount = indexed_.primaryCount;
			return *existsByMeta;
		}

		return set_.emplace_back(std::move(indexed_));
	}

	if (indexed_.primary.empty() && not indexed_.meta.empty()) {
		::hg::assertrt(existsByMeta == set_.end());

		if (existsByPrimary != set_.end()) {
			existsByPrimary->meta = indexed_.meta;
			return *existsByPrimary;
		}

		return set_.emplace_back(std::move(indexed_));
	}

	/**/

	::panic();
}

/**/

static void indexDirectory(
	_In_ cref<path> path_,
	_Inout_ ref<Vector<path>> backlog_,
	_Inout_ ref<Vector<Indexed>> indexSet_
);

static void autoIndex(_In_ cref<path> root_);

/**/

void indexDirectory(cref<path> path_, ref<Vector<path>> backlog_, ref<Vector<Indexed>> indexSet_) {

	auto range = std::filesystem::directory_iterator {
		path_, std::filesystem::directory_options::follow_directory_symlink
	};

	for (const auto& candidate : range) {

		buildIndexedFrom(candidate.path()).map_or_else(
			[&indexSet_](mref<Indexed> indexed_) {
				upsertIndexed(indexSet_, std::move(indexed_));
			},
			[&backlog_, &candidate]() {
				backlog_.emplace_back(candidate.path());
			}
		);

	}
}

void autoIndex(cref<path> root_) {

	if (not std::filesystem::exists(root_)) {
		return;
	}

	Vector<path> backlog {};
	Vector<Indexed> indexSet {};

	backlog.emplace_back(root_);
	while (not backlog.empty()) {

		const auto cur { backlog.back() };
		backlog.pop_back();

		indexDirectory(cur, backlog, indexSet);

		auto rit = std::ranges::remove_if(
			indexSet,
			[](auto& data_) {
				return not data_.meta.empty() && data_.scheduled.has_value() && data_.scheduled.value().ready();
			}
		);
		indexSet.erase(rit.begin(), rit.end());
	}

	/**/

	auto drop = [](ref<Indexed> indexed_) {
		const auto primaryExists = not indexed_.primary.empty();
		const auto metaExists = not indexed_.meta.empty();

		if (not primaryExists && not metaExists) {
			IM_CORE_WARNF("Encountered auto-indexed object without meta or primary path.");
			return true;
		}

		if (primaryExists && metaExists) {
			indexed_.scheduled = enqueueIndexedLoad(indexed_);
			IM_DEBUG_LOGF("Enqueued indexed loading of `{}`.", indexed_.primary.string());
			return false;
		}

		if (primaryExists && not metaExists) {

			if (auto_import(indexed_)) {
				// Note: We currently don't schedule async, so we don't need to wait
				return true;
			}

			IM_CORE_WARNF("Failed to auto-import `{}`.", indexed_.primary.string());
			return true;
		}

		// IM_CORE_WARNF("Encountered auto-indexed meta file without an associated primary object.");
		return true;
	};

	/**/

	auto remove = std::ranges::remove_if(indexSet, std::move(drop));
	indexSet.erase(remove.begin(), remove.end());

	/**/

	for (auto&& indexed : indexSet) {
		if (indexed.scheduled) {
			await(std::move(indexed.scheduled)->signal().get());
		}
	}
}

#pragma endregion

void editor::boot::initAssets() {

	const auto& cfg = engine::Engine::getEngine()->getConfig();

	const auto& editorAssetPath = cfg.getTyped<String>(engine::cfg::EditorConfigProperty::eLocalAssetPath);
	::hg::assertrt(editorAssetPath.has_value() && editorAssetPath->has_value());

	const auto& projectAssetPath = cfg.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalAssetPath);
	::hg::assertrt(projectAssetPath.has_value() && projectAssetPath->has_value());

	autoIndex(**editorAssetPath);
	autoIndex(**projectAssetPath);
}

void editor::boot::initDefaultAssets() {

	const auto& engine = *engine::Engine::getEngine();

	initDefaultImageAssets(engine);
	initDefaultTextureAssets(engine);
	initDefaultMeshAssets(engine);

	initDefaultFontAssets(engine);
	initDefaultGfxMaterialAssets(engine);
}

/**/

#include <Engine.Reflect/Reflect.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

void package_discover_assets(mref<Arci<storage::system::PackageStorage>> packStore_) {

	const auto engine = Engine::getEngine();
	const auto storeSys = engine->getStorage()->getSystem();

	// TODO: We need some iterator over the storages by a certain filter ( package storages in this case )
	// Question: Do we need a global lock to iterate the stored storage objects?

	// TODO: Check whether we want an option to filter for a certain (raw, structured, layout) arch-type?
	auto maybeEnumArchives = storeSys->enumArchivesFromPackage(::hg::move(packStore_));

	/* Check for stored archives */

	if (not maybeEnumArchives.has_value() || maybeEnumArchives.value().empty()) {
		return;
	}

	/* Index / Load data from archives */

	for (auto&& archive : maybeEnumArchives.value()) {
		// @formatter:off
		const auto storageUrl = storage::ArchiveUrl { archive->getArchiveGuid() };
		storeSys->query(::hg::move(archive), [&storageUrl](_In_ ref<resource::StorageReadonlyArchive> archive_) {
			// @formatter:on

			if (archive_.type() != resource::ArchiveType::eSerializedStructure) {
				return;
			}

			auto structured = serialization::StructuredArchive { archive_ };

			/* Check for root stored assets */

			if (is_archived_asset(structured.getRootSlot())) {

				archive_.seek(0LL);
				try_load_archived_asset(storageUrl, structured.getRootSlot());
				return;
			}

			/* Check for a sequence of stored assets */

			{
				archive_.seek(0);
				auto root = structured.getRootSlot();

				root.slot()->readHeader();
				if (root.slot()->getSlotHeader().type != serialization::StructureSlotType::eSeq) {
					return;
				}

				/* Check whether first element is asset, otherwise treat as unknown sequence */

				const auto seq = std::move(root).intoSeq();
				const auto count = seq.getRecordCount();

				if (count <= 0) {
					return;
				}

				if (not is_archived_asset(seq.getRecordSlot(0))) {
					return;
				}

				/* Iterate over archives and try to load */

				for (s64 idx = 0; idx < count; ++idx) {
					try_load_archived_asset(storageUrl, seq.getRecordSlot(idx));
				}
			}

			// @formatter:off
		});
		// @formatter:on
	}
}

bool is_archived_asset(mref<serialization::RecordScopedSlot> record_) {

	const auto record = std::move(record_).intoStruct();
	if (not record.slot()->validateType()) {
		return false;
	}

	/**/

	if (not record.hasRecordSlot("__type__")) {
		return false;
	}

	if (not record.hasRecordSlot("__guid__")) {
		return false;
	}

	/**/

	AssetTypeId typeId {};
	AssetGuid guid = invalid_asset_guid;

	serialization::access::Structure<Guid>::hydrate(record.getStructSlot("__guid__"), guid);
	record.getSlot<u64>("__type__") >> typeId.data;

	/**/

	if (guid == invalid_asset_guid || typeId.data == 0) {
		return false;
	}

	// TODO: Lookup type id

	return true;
}

bool try_load_archived_asset(ref<const storage::ArchiveUrl> assetStorageUrl_, mref<serialization::RecordScopedSlot> record_) {

	auto record = record_.asStruct();

	/**/

	AssetTypeId typeId {};
	AssetGuid guid = invalid_asset_guid;

	serialization::access::Structure<Guid>::hydrate(record.getStructSlot("__guid__"), guid);
	record.getSlot<u64>("__type__") >> typeId.data;

	/**/

	auto genericLoad = []<std::derived_from<assets::Asset> AssetType_>(
		ref<const storage::ArchiveUrl> archiveStorageUrl_,
		AssetGuid guid_,
		mref<serialization::StructScopedSlot> record_
	) -> Opt<Arci<AssetType_>> {

		const auto registry = Engine::getEngine()->getAssets()->getRegistry();
		const auto asset = registry->findAssetByGuid(guid_);

		if (asset != None) { return None; }

		/**/

		auto nextAsset = Arci<AssetType_>::create();
		serialization::access::Structure<AssetType_>::hydrate(std::move(record_), *nextAsset);

		nextAsset->setAssetStorageUrl(clone(archiveStorageUrl_));

		/**/

		if (nextAsset->getAssetVfsUrl().empty() && not try_recover_vfs(
			*nextAsset,
			archiveStorageUrl_
		)) {
			IM_CORE_ERRORF(
				"Failed to recover virtual filesystem url for asset (`{}` -> `{}`).",
				nextAsset->getAssetName(),
				encodeGuid4228(guid_)
			);
			nextAsset.reset();
			return None;
		}

		/**/

		auto success = false;
		auto hard_failure = false;
		Opt<RetryContext> retry_context = None;
		// TODO: Add retry context to track e.g. the renaming suffix

		do {

			if (retry_context != None) {
				auto next_asset_name = std::format("{} ({})", retry_context->baseName, retry_context->retry);
				nextAsset = assets::rename(
					::hg::move(nextAsset).template into<assets::Asset>(),
					::hg::move(next_asset_name)
				).template into<AssetType_>();
				++retry_context->retry;
			}

			/**/

			const auto result = registry->insertOrFail({ clone(nextAsset).template into<assets::Asset>() });

			/**/

			success = result.has_value();
			hard_failure = not success && not result.error().template is<assets::system::IndexSoftFailure>();

			if (not success && not hard_failure && not retry_context) {
				IM_CORE_WARNF(
					"Tried to load asset (`{}` -> `{}`) from package, but need to refactor asset, as it's already used.",
					nextAsset->getAssetName(),
					encodeGuid4228(guid_)
				);

				retry_context = Some(RetryContext { .baseName = nextAsset->getAssetName(), .retry = 1uLL });
			}

		} while (not success && not hard_failure);

		/**/

		if (not success && hard_failure) {
			IM_CORE_WARNF(
				"Tried to load asset (`{}` -> `{}`) from package, but it is already present.",
				nextAsset->getAssetName(),
				encodeGuid4228(guid_)
			);
			::hg::breakpoint();
			nextAsset.reset();
			return None;
		}

		IM_CORE_LOGF(
			"Successfully loaded asset `{}` -> `{}` -> `{}` from package.",
			nextAsset->getAssetName(),
			StringView { nextAsset->getAssetVfsUrl().getAssetPath().asByteSpan() },
			nextAsset->getAssetStorageUrl().encode()
		);
		return Some(std::move(nextAsset));
	};

	switch (typeId.data) {
		case assets::FontAsset::typeId.data: {
			return genericLoad.operator()<assets::FontAsset>(assetStorageUrl_, guid, std::move(record)).has_value();
		}
		case assets::ImageAsset::typeId.data: {
			return genericLoad.operator()<assets::ImageAsset>(assetStorageUrl_, guid, std::move(record)).has_value();
		}
		case assets::TextureAsset::typeId.data: {
			return genericLoad.operator()<assets::TextureAsset>(assetStorageUrl_, guid, std::move(record)).has_value();
		}
		case assets::StaticGeometry::typeId.data: {
			return genericLoad.operator()<assets::StaticGeometry>(assetStorageUrl_, guid, std::move(record)).has_value();
		}
		default: {
			return false;
		}
	}

	std::unreachable();
}

bool auto_import(ref<Indexed> indexed_) {

	const auto actions = nmpt<editor::ActionManager> {
		static_cast<ptr<editor::ActionManager>>(
			Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey).get()
		)
	};

	/**/

	auto importAction = Arci<editor::AutoImportAction>::create(
		storage::FileUrl { clone(storage::FileScheme), engine::fs::Path { indexed_.primary } }
	);
	const auto result = actions->apply(clone(importAction));

	/**/

	return result.has_value();
}

bool try_recover_vfs(ref<assets::Asset> asset_, ref<const storage::ArchiveUrl> archiveUrl_) {

	const auto& system = *Engine::getEngine()->getStorage()->getSystem();

	const auto archiveStorage = system.findArchive(clone(archiveUrl_));
	if (archiveStorage == None) {
		return false;
	}

	auto archiveBackingStorage = (*archiveStorage)->getBacking();
	if (archiveBackingStorage == nullptr || (
		not IsType<storage::system::PackageStorage>(*archiveBackingStorage) &&
		not IsType<storage::system::LocalFileStorage>(*archiveBackingStorage)
	)) {
		return false;
	}

	auto lfsStorage = Arci<storage::system::LocalFileStorage> {};
	if (IsType<storage::system::LocalFileStorage>(*archiveBackingStorage)) {
		lfsStorage = ::hg::move(archiveBackingStorage).into<storage::system::LocalFileStorage>();

	} else {
		const auto packageStorage = ::hg::move(archiveBackingStorage).into<storage::system::PackageStorage>();
		auto packageBackingStorage = packageStorage->getBacking();

		if (packageBackingStorage == nullptr || not IsType<storage::system::LocalFileStorage>(*packageBackingStorage)) {
			return false;
		}

		lfsStorage = ::hg::move(packageBackingStorage).into<storage::system::LocalFileStorage>();
	}

	/**/

	auto storageLfsPath = clone(lfsStorage->_lfsPath);
	::hg::assertd(std::filesystem::exists(storageLfsPath));

	// Note: This is a lot of guessing, as we try to find a suitable config path mapping and the shortest relative
	// Note:	sub-path to artificially re-locate the asset.
	auto vfsPathCandidate = find_best_match_relative(storageLfsPath.normalized());
	if (vfsPathCandidate == None) {
		return false;
	}

	// Note: We will use the full lfs path, instead of the stripped version, trying to prevent data-raises due to
	// Note:	naming conflicts. (Same as 'Image' and 'Texture' suffix naming for texture imports)
	// Warning: This will only work as sanitization when used with un-bundled asset archives
	asset_.setAssetVfsUrl(assets::AssetUrl { ::hg::move(vfsPathCandidate).value(), asset_.getAssetName() });
	return true;
}

Opt<::hg::engine::fs::Path> find_best_match_relative(ref<const ::hg::engine::fs::Path> lfsStoragePath_) {

	const auto& cfg = engine::Engine::getEngine()->getConfig();

	/**/

	const auto& cfgEditorAssetPath = cfg.getTyped<String>(engine::cfg::EditorConfigProperty::eLocalAssetPath);
	::hg::assertrt(cfgEditorAssetPath.has_value() && cfgEditorAssetPath->has_value());
	const auto editorAssetPath = engine::fs::Path { **cfgEditorAssetPath }.normalized();

	Opt<engine::fs::Path> result = None;
	if (lfsStoragePath_.isSubPathOf(editorAssetPath)) {
		result = Some(engine::fs::Path { std::filesystem::relative(lfsStoragePath_, editorAssetPath) });
	}

	/**/

	const auto& cfgProjectAssetPath = cfg.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalAssetPath);
	::hg::assertrt(cfgProjectAssetPath.has_value() && cfgProjectAssetPath->has_value());
	const auto projectAssetPath = engine::fs::Path { **cfgProjectAssetPath }.normalized();

	if (lfsStoragePath_.isSubPathOf(projectAssetPath)) {
		auto next = engine::fs::Path { std::filesystem::relative(lfsStoragePath_, projectAssetPath) };
		if (result == None || (*result <=> next) == std::strong_ordering::greater) {
			result = Some(engine::fs::Path { ::hg::move(next) });
		}
	}

	return result;
}
