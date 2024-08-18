#include "AssetInit.hpp"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Heliogrim/Heliogrim.hpp>

#include "Editor.Action/ActionManager.hpp"
#include "Editor.Action/Action/Import/AutoImportAction.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultBrdfMaterial.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultBrdfMaterialPrototype.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultSkybox.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultSkyboxPrototype.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets/Types/Font.hpp"
#include "Engine.Assets/Types/Image.hpp"
#include "Engine.Assets/Types/Geometry/StaticGeometry.hpp"
#include "Engine.Assets/Types/Texture/TextureAsset.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.Resource.Archive/BufferArchive.hpp"
#include "Engine.Resource.Archive/StorageReadonlyArchive.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Resource.Package/Attribute/MagicBytes.hpp"
#include "Engine.Resource.Package/Attribute/PackageGuid.hpp"
#include "Engine.Resource.Package/External/PackageIo.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"
#include "Engine.Storage/IStorage.hpp"
#include "Engine.Storage/IStorageRegistry.hpp"
#include "Engine.Storage/StorageModule.hpp"
#include "Engine.Storage/Options/FileStorageDescriptor.hpp"
#include "Engine.Storage/Options/PackageStorageDescriptor.hpp"
#include "Engine.Storage/Options/StorageDescriptor.hpp"
#include "Engine.Storage/Storage/LocalFileStorage.hpp"
#include "Engine.Storage/Storage/PackageStorage.hpp"

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

/**/

// @formatter:off
static void packageDiscoverAssets(mref<Arci<storage::system::PackageStorage>> packStore_);
static bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_);
static bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_);
static bool autoImport(ref<Indexed> indexed_);
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
	const auto storeReg = engine->getStorage()->getRegistry();
	auto storeIo = engine->getStorage()->getIo();
	auto packIo = storage::PackageIo { *storeIo };

	/**/

	auto storeMetaFile = storeReg->insert(
		storage::FileStorageDescriptor {
			storage::FileUrl {
				clone(storage::FileProjectScheme),
				engine::fs::Path { indexed_.meta }
			}
		}
	);

	/**/

	auto accessMetaFile = storeIo->accessReadonlyBlob(storeMetaFile.into<storage::system::LocalFileStorage>());
	if (not packIo.isPackageBlob(accessMetaFile)) {
		IM_CORE_ERRORF(
			"Encountered indexed meta file `{}` which is not identifiable as package upon inspection.",
			indexed_.meta.string()
		);
		return None;
	}

	/**/

	auto package = packIo.create_package_from_storage(accessMetaFile);

	auto storePack = storeReg->insert(
		storage::PackageStorageDescriptor {
			storage::PackageUrl { resource::PackageGuid::ntoh(package->header().guid) },
			std::move(storeMetaFile)
		}
	).into<storage::system::PackageStorage>();

	/**/

	// @formatter:off
	return Some(scheduler::async<void>([storePack = std::move(storePack)]() mutable {
		packageDiscoverAssets(std::move(storePack));
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

			if (autoImport(indexed_)) {
				// Note: We currently don't schedule async, so we don't need to wait
				return true;
			}

			IM_CORE_WARNF("Failed to auto-import `{}`.", indexed_.primary.string());
			return true;
		}

		IM_CORE_WARNF("Encountered auto-indexed meta file without an associated primary object.");
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

#pragma region Predefined Loading

static void initMaterialDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	/**/

	delete(new(material::DefaultBrdfMaterialPrototype));
	delete(new(material::DefaultBrdfMaterial));
}

static void initSkyboxDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	/**/

	delete(new(material::DefaultSkyboxPrototype));
	delete(new(material::DefaultSkybox));
}

#pragma endregion

void editor::boot::initAssets() {

	const auto relative = std::filesystem::current_path().append(R"(../../../assets)");
	if (not std::filesystem::exists(relative)) {
		std::filesystem::create_directories(relative);
	}
	auto root = std::filesystem::canonical(relative);

	autoIndex(root);

	/**/

	initMaterialDefaults();
	initSkyboxDefaults();
}

/**/

#include <Engine.Reflect/Reflect.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchive.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

void packageDiscoverAssets(mref<Arci<storage::system::PackageStorage>> packStore_) {

	auto storeIo = Engine::getEngine()->getStorage()->getIo();
	auto packIo = storage::PackageIo { *storeIo };

	// TODO: We need some iterator over the storages by a certain filter ( package storages in this case )
	// Question: Do we need a global lock to iterate the stored storage objects?

	auto packAccess = storeIo->accessReadonly(std::move(packStore_));
	auto linker = packIo.create_linker_from_package(packAccess);

	/* Check for stored archives */

	if (linker->getArchiveCount() <= 0) {
		return;
	}

	/* Index / Load data from archives */

	const auto end = linker->cend();
	for (auto iter = linker->cbegin(); iter != end; ++iter) {

		if (iter.header().type != resource::package::PackageArchiveType::eSerializedStructure) {
			continue;
		}

		auto archive = iter.archive();
		auto structured = serialization::StructuredArchive { *archive };

		/* Check for root stored assets */

		if (isArchivedAsset(structured.getRootSlot())) {

			archive->seek(0LL);
			tryLoadArchivedAsset(structured.getRootSlot());

			continue;
		}

		/* Check for a sequence of stored assets */

		{
			archive->seek(0);
			auto root = structured.getRootSlot();

			root.slot()->readHeader();
			if (root.slot()->getSlotHeader().type != serialization::StructureSlotType::eSeq) {
				continue;
			}

			/* Check whether first element is asset, otherwise treat as unknown sequence */

			const auto seq = root.intoSeq();
			const auto count = seq.getRecordCount();

			if (count <= 0) {
				continue;
			}

			if (not isArchivedAsset(seq.getRecordSlot(0))) {
				continue;
			}

			/* Iterate over archives and try to load */

			for (s64 idx = 0; idx < count; ++idx) {
				tryLoadArchivedAsset(seq.getRecordSlot(idx));
			}
		}
	}
}

bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_) {

	const auto record = record_.intoStruct();
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

	asset_type_id typeId {};
	asset_guid guid = invalid_asset_guid;

	serialization::access::Structure<Guid>::deserialize(&guid, record.getRecordSlot("__guid__"));
	record.getSlot<u64>("__type__") >> typeId.data;

	/**/

	if (guid == invalid_asset_guid || typeId.data == 0) {
		return false;
	}

	// TODO: Lookup type id

	return true;
}

bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_) {

	const auto record = record_.asStruct();

	/**/

	asset_type_id typeId {};
	asset_guid guid = invalid_asset_guid;

	serialization::access::Structure<Guid>::deserialize(&guid, record.getRecordSlot("__guid__"));
	record.getSlot<u64>("__type__") >> typeId.data;

	/**/

	auto genericLoad = []<class AssetType_>(
		asset_guid guid_,
		mref<serialization::RecordScopedSlot> record_
	) -> Opt<Arci<AssetType_>> {

		const auto registry = Engine::getEngine()->getAssets()->getRegistry();
		const auto asset = registry->findAssetByGuid(guid_);

		if (asset != nullptr) { return None; }

		/**/

		auto nextAsset = Arci<AssetType_>::template create();
		serialization::access::Structure<AssetType_>::deserialize(nextAsset.get(), std::move(record_));

		engine::assets::storeDefaultNameAndUrl(*nextAsset, {});
		const auto succeeded = registry->insert({ clone(nextAsset).template into<assets::Asset>() });

		/**/

		if (not succeeded) {
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
			"Successfully loaded asset `{}` -> `{}` from package.",
			nextAsset->getAssetName(),
			nextAsset->getVirtualUrl()
		);
		return Some(std::move(nextAsset));
	};

	switch (typeId.data) {
		case assets::Font::typeId.data: {
			return genericLoad.operator()<assets::Font>(guid, std::move(record_)).has_value();
		}
		case assets::Image::typeId.data: {
			return genericLoad.operator()<assets::Image>(guid, std::move(record_)).has_value();
		}
		case assets::TextureAsset::typeId.data: {
			return genericLoad.operator()<assets::TextureAsset>(guid, std::move(record_)).has_value();
		}
		case assets::StaticGeometry::typeId.data: {
			return genericLoad.operator()<assets::StaticGeometry>(guid, std::move(record_)).has_value();
		}
		default: {
			return false;
		}
	}

	std::unreachable();
}

bool autoImport(ref<Indexed> indexed_) {

	const auto actions = nmpt<editor::ActionManager> {
		static_cast<ptr<editor::ActionManager>>(
			Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey).get()
		)
	};

	/**/

	auto importAction = make_sptr<editor::AutoImportAction>(
		storage::FileUrl { clone(storage::FileScheme), engine::fs::Path { indexed_.primary } }
	);
	actions->apply(importAction);

	/**/

	return not importAction->failed();
}
