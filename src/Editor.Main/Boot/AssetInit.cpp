#include "AssetInit.hpp"

#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Heliogrim/Heliogrim.hpp>

#include <filesystem>
#include <fstream>
#include <Engine.Pedantic/Clone/Clone.hpp>

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
#include "Engine.Resource.Archive/BufferArchive.hpp"
#include "Engine.Resource.Archive/StorageReadonlyArchive.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Resource.Package/Attribute/PackageGuid.hpp"
#include "Engine.Resource.Package/External/PackageIo.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"
#include "Engine.Storage/IStorageRegistry.hpp"
#include "Engine.Storage/StorageModule.hpp"
#include "Engine.Storage/IStorage.hpp"
#include "Engine.Storage/Storage/PackageStorage.hpp"
#include "Engine.Storage/Storage/LocalFileStorage.hpp"
#include "Engine.Storage/Options/FileStorageDescriptor.hpp"
#include "Engine.Storage/Options/PackageStorageDescriptor.hpp"
#include "Engine.Storage/Options/StorageDescriptor.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

using path = std::filesystem::path;

namespace hg::engine::serialization {
	class RecordScopedSlot;
}

/**/
static void packageDiscoverAssets(mref<Arci<storage::system::PackageStorage>> packStore_);

static bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_);

static bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_);

/**/

#pragma region File System Loading

static void dispatchLoad(cref<path> path_) {
	scheduler::exec(
		[file = path_]() {
			// TODO:

			constexpr auto minLength = sizeof(asset_guid) + sizeof(asset_type_id);

			std::ifstream ifs { file, std::ios::in | std::ios::binary };
			ifs.seekg(0, std::ios::beg);

			resource::BufferArchive archive {};
			archive.resize(minLength, {});

			auto start { ifs.tellg() };
			ifs.read(reinterpret_cast<char*>(archive.data()), archive.size());
			auto end { ifs.tellg() };

			if (ifs.bad()) {
				// Can't even read asset guid or type id, so no viable data
				return;
			}

			const auto length = end - start;
			if (length < minLength) {
				// Can't even read asset guid or type id, so no viable data
				return;
			}

			/**/

			asset_guid possibleGuid { invalid_asset_guid };
			asset_type_id possibleTypeId {};

			// TODO: Use predefined data layouts to deserialize guid and type id, cause they are not representable as u64 at every platform
			archive >> reinterpret_cast<ref<u64>>(possibleGuid);
			archive >> reinterpret_cast<ref<u64>>(possibleTypeId);

			if (possibleGuid == invalid_asset_guid) {
				return;
			}

			if (possibleTypeId.data == 0) {
				return;
			}

			/**/

			const auto& layouts = serialization::LayoutManager::get();
			const auto layout = layouts.getLayout(possibleTypeId);

			if (not layout || not layout->hasClass()) {
				// Seams not to be a valid data package, asset type was not bootstrapped correctly or asset is not generically reconstructable
				return;
			}

			/**/

			ifs.seekg(0, std::ios::end);
			end = ifs.tellg();

			ifs.seekg(0, std::ios::beg);
			start = ifs.tellg();

			archive.seek(0);
			archive.resize(end - start, {});

			ifs.read(reinterpret_cast<ptr<char>>(archive.data()), archive.size());

			if (ifs.bad()) {
				// Something went wrong while reading the actual data
				IM_CORE_ERRORF("Received an error while reading possible asset file `{}`", file.string());
				return;
			}

			/**/

			::hg::todo_panic();

			// Warning: Unsafe operation, should be capsulated within io system
			auto* obj { layout->reflect().instantiate() };
			const std::span<u8, std::dynamic_extent> view {
				reinterpret_cast<ptr<u8>>(obj),
				static_cast<u64>(layout->size())
			};

			layout->dispatch().load(archive, view);

			/**/

			//auto* asset { static_cast<ptr<engine::assets::Asset>>(obj) };
			//Engine::getEngine()->getAssets()->getRegistry()->insert({ asset });
		}
	);
}

static Optional<concurrent::future<void>> tryDispatchLoad(cref<path> path_) {

	const auto extension = path_.extension().string();

	if (extension.empty()) {
		return tl::nullopt;
	}

	const auto reg = Engine::getEngine()->getStorage()->getRegistry();
	auto io = Engine::getEngine()->getStorage()->getIo();
	auto packIo = storage::PackageIo { *io };

	if (extension.ends_with(".imp") || extension.ends_with(".impackage")) {

		// TODO: Rework
		auto storeFileUrl = storage::FileUrl { clone(storage::FileScheme), engine::fs::Path { path_ } };
		auto storeFile = reg->insert(storage::FileStorageDescriptor { clone(storeFileUrl) });

		// Validate
		if (not packIo.isPackageFile(storeFileUrl, storeFile)) {
			IM_CORE_WARNF(
				"Encountered file `{}` which is not identifiable as package upon inspection.",
				path_.string()
			);
			return tl::nullopt;
		}

		// Question: How to communicate that we want to load the package from backing storage internally?
		auto accessFile = io->accessReadonlyBlob(storeFile.into<storage::system::LocalFileStorage>());
		auto package = packIo.create_package_from_storage(accessFile);

		auto storePackUrl = storage::PackageUrl { resource::PackageGuid::ntoh(package->header().guid) };
		auto storePack = reg->insert(
			storage::PackageStorageDescriptor { std::move(storePackUrl), std::move(storeFile) }
		).into<storage::system::PackageStorage>();

		/* Discover data within package */
		return tl::make_optional(
			scheduler::async<void>(
				[storePack = std::move(storePack)]()mutable {
					packageDiscoverAssets(std::move(storePack));
				}
			)
		);

	} else if (extension.ends_with(".ima") || extension.ends_with(".imasset")) {

		dispatchLoad(path_);
	}

	return tl::nullopt;
}

static void indexLoadable(
	_In_ cref<path> path_,
	_Inout_ ref<Vector<path>> backlog_,
	_Inout_ ref<Vector<concurrent::future<void>>> awaits_
) {

	const auto directory {
		std::filesystem::directory_iterator { path_, std::filesystem::directory_options::follow_directory_symlink }
	};

	for (const auto& entry : directory) {

		if (entry.is_directory()) {
			backlog_.push_back(entry.path());
			continue;
		}

		if (entry.is_regular_file()) {
			auto mayAsync = tryDispatchLoad(entry.path());
			if (mayAsync.has_value()) {
				awaits_.emplace_back(std::move(mayAsync.value()));
				mayAsync.reset();
			}
			//continue;
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

	switch (typeId.data) {
		case assets::Image::typeId.data: {

			const auto registry = Engine::getEngine()->getAssets()->getRegistry();
			const auto asset = registry->findAssetByGuid(guid);

			if (asset != nullptr) {
				return false;
			}

			/**/

			auto image = Arci<assets::Image>::create();
			serialization::access::Structure<assets::Image>::deserialize(image.get(), std::move(record_));

			engine::assets::storeDefaultNameAndUrl(*image, {});
			const auto succeeded = registry->insert({ clone(image).into<assets::Asset>() });

			if (not succeeded) {

				IM_CORE_WARNF(
					"Tried to load Image (`{}` -> `{}`) from package, but it is already present.",
					image->getAssetName(),
					encodeGuid4228(guid)
				);

				__debugbreak();
				image.reset();

			} else {
				IM_CORE_LOGF(
					"Successfully loaded Image (`{}` -> `{}`) from package.",
					image->getAssetName(),
					encodeGuid4228(guid)
				);
			}

			/**/

			break;
		}
		case assets::TextureAsset::typeId.data: {

			const auto registry = Engine::getEngine()->getAssets()->getRegistry();
			const auto asset = registry->findAssetByGuid(guid);

			if (asset != nullptr) {
				return false;
			}

			/**/

			auto texture = Arci<assets::TextureAsset>::create();
			serialization::access::Structure<assets::TextureAsset>::deserialize(texture.get(), std::move(record_));

			engine::assets::storeDefaultNameAndUrl(*texture, {});
			const auto succeeded = registry->insert({ clone(texture).into<assets::Asset>() });

			if (not succeeded) {

				IM_CORE_WARNF(
					"Tried to load Texture (`{}` -> `{}`) from package, but it is already present.",
					texture->getAssetName(),
					encodeGuid4228(guid)
				);

				__debugbreak();
				texture.reset();

			} else {
				IM_CORE_LOGF(
					"Successfully loaded Texture (`{}` -> `{}`) from package.",
					texture->getAssetName(),
					encodeGuid4228(guid)
				);
			}

			/**/

			break;
		}
		case assets::StaticGeometry::typeId.data: {

			const auto registry = Engine::getEngine()->getAssets()->getRegistry();
			const auto asset = registry->findAssetByGuid(guid);

			if (asset != nullptr) {
				return false;
			}

			/**/

			auto geom = Arci<assets::StaticGeometry>::create();
			serialization::access::Structure<assets::StaticGeometry>::deserialize(geom.get(), std::move(record_));

			engine::assets::storeDefaultNameAndUrl(*geom, {});
			const auto succeeded = registry->insert({ clone(geom).into<assets::Asset>() });

			if (not succeeded) {

				IM_CORE_WARNF(
					"Tried to load Static Geometry (`{}` -> `{}`) from package, but it is already present.",
					geom->getAssetName(),
					encodeGuid4228(guid)
				);

				__debugbreak();
				geom.reset();

			} else {
				IM_CORE_LOGF(
					"Successfully loaded Static Geometry (`{}` -> `{}`) from package.",
					geom->getAssetName(),
					encodeGuid4228(guid)
				);
			}

			/**/

			break;
		}
	}

	return true;
}
