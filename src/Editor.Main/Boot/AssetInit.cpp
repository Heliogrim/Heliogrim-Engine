#include "AssetInit.hpp"

#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Heliogrim/Heliogrim.hpp>

#include <filesystem>
#include <fstream>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Editor.Assets.Default/Fonts/CascadiaCode.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultBrdfMaterial.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultBrdfMaterialPrototype.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultSkybox.hpp"
#include "Editor.Assets.Default/GfxMaterials/DefaultSkyboxPrototype.hpp"
#include "Editor.Assets.Default/Images/Brand.hpp"
#include "Editor.Assets.Default/Images/DefaultAlpha.hpp"
#include "Editor.Assets.Default/Images/DefaultAO.hpp"
#include "Editor.Assets.Default/Images/DefaultDiffuse.hpp"
#include "Editor.Assets.Default/Images/DefaultMetalness.hpp"
#include "Editor.Assets.Default/Images/DefaultNormal.hpp"
#include "Editor.Assets.Default/Images/DefaultRoughness.hpp"
#include "Editor.Assets.Default/Images/DefaultSkybox.hpp"
#include "Editor.Assets.Default/Images/Directory.hpp"
#include "Editor.Assets.Default/Images/FolderAudio.hpp"
#include "Editor.Assets.Default/Images/FolderEnvironment.hpp"
#include "Editor.Assets.Default/Images/FolderFont.hpp"
#include "Editor.Assets.Default/Images/FolderImages.hpp"
#include "Editor.Assets.Default/Images/FolderImport.hpp"
#include "Editor.Assets.Default/Images/FolderLog.hpp"
#include "Editor.Assets.Default/Images/FolderResource.hpp"
#include "Editor.Assets.Default/Images/FolderShader.hpp"
#include "Editor.Assets.Default/Images/FolderVideo.hpp"
#include "Editor.Assets.Default/Meshes/Sphere.hpp"
#include "Editor.Assets.Default/Textures/DefaultAlpha.hpp"
#include "Editor.Assets.Default/Textures/DefaultAO.hpp"
#include "Editor.Assets.Default/Textures/DefaultDiffuse.hpp"
#include "Editor.Assets.Default/Textures/DefaultMetalness.hpp"
#include "Editor.Assets.Default/Textures/DefaultNormal.hpp"
#include "Editor.Assets.Default/Textures/DefaultRoughness.hpp"
#include "Editor.Assets.Default/Textures/DefaultSkybox.hpp"
#include "Editor.Assets.Default/Textures/Directory.hpp"
#include "Editor.Assets.Default/Textures/FolderAudio.hpp"
#include "Editor.Assets.Default/Textures/FolderEnvironment.hpp"
#include "Editor.Assets.Default/Textures/FolderFont.hpp"
#include "Editor.Assets.Default/Textures/FolderImages.hpp"
#include "Editor.Assets.Default/Textures/FolderImport.hpp"
#include "Editor.Assets.Default/Textures/FolderLog.hpp"
#include "Editor.Assets.Default/Textures/FolderResource.hpp"
#include "Editor.Assets.Default/Textures/FolderShader.hpp"
#include "Editor.Assets.Default/Textures/FolderVideo.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Assets/Types/Font.hpp"
#include "Engine.Assets/Types/Image.hpp"
#include "Engine.Assets/Types/Geometry/StaticGeometry.hpp"
#include "Engine.Assets/Types/Texture/TextureAsset.hpp"
#include "Engine.Resource/Source/FileSource.hpp"
#include "Engine.Resource.Package/PackageFactory.hpp"
#include "Engine.Resource.Package/PackageManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Resource.Package/Linker/PackageLinker.hpp"
#include "Engine.Serialization/Layout/DataLayoutBase.hpp"
#include "Engine.Serialization.Layouts/LayoutManager.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

using path = std::filesystem::path;

namespace hg::engine::serialization {
	class RecordScopedSlot;
}

/**/
void packageDiscoverAssets();

bool isArchivedAsset(mref<serialization::RecordScopedSlot> record_);

bool tryLoadArchivedAsset(mref<serialization::RecordScopedSlot> record_);

/**/

#pragma region File System Loading

void dispatchLoad(cref<path> path_) {
	scheduler::exec(
		[file = path_]() {
			// TODO:

			constexpr auto minLength = sizeof(asset_guid) + sizeof(asset_type_id);

			std::ifstream ifs { file, std::ios::in | std::ios::binary };
			ifs.seekg(0, std::ios::beg);

			serialization::BufferArchive archive {};
			archive.resize(minLength, 0u);

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
			archive.resize(end - start, 0u);

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

void tryDispatchLoad(cref<path> path_) {

	const auto extension = path_.extension().string();

	if (extension.empty()) {
		return;
	}

	auto* const pm = Engine::getEngine()->getResources()->packages(traits::nothrow);
	hg::fs::File file { path_ };

	if (pm->isPackageFile(file)) {

		// TODO: Load Package from file
		auto source = make_uptr<resource::FileSource>(std::move(file));
		auto package = resource::PackageFactory::createFromSource(std::move(source));

		/**/

		auto managed = make_smr<resource::UniqueResource<resource::PackageResource::value_type>>(package.release());

		/**/

		pm->addPackage(std::move(managed));

	} else if (extension.ends_with(".ima") || extension.ends_with(".imasset")) {

		dispatchLoad(path_);
	}
}

void indexLoadable(cref<path> path_, ref<Vector<path>> backlog_) {

	const auto directory {
		std::filesystem::directory_iterator { path_, std::filesystem::directory_options::follow_directory_symlink }
	};

	for (const auto& entry : directory) {

		if (entry.is_directory()) {
			backlog_.push_back(entry.path());
			continue;
		}

		if (entry.is_regular_file()) {
			tryDispatchLoad(entry.path());
			//continue;
		}
	}
}

#pragma endregion

#pragma region Predefined Loading

static void initMaterialDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createImageAsset(
		clone(image::DefaultAlpha::unstable_auto_guid()),
		R"(resources\imports\ktx\default_alpha.ktx)"
	);

	factory->createImageAsset(
		clone(image::DefaultAO::unstable_auto_guid()),
		R"(resources\imports\ktx\default_ao.ktx)"
	);

	factory->createImageAsset(
		image::DefaultDiffuse::auto_guid(),
		R"(resources\imports\ktx\default_diffuse.ktx)"
	);

	factory->createImageAsset(
		clone(image::DefaultMetalness::unstable_auto_guid()),
		R"(resources\imports\ktx\default_metalness.ktx)"
	);

	factory->createImageAsset(
		clone(image::DefaultNormal::unstable_auto_guid()),
		R"(resources\imports\ktx\default_normal.ktx)"
	);

	factory->createImageAsset(
		clone(image::DefaultRoughness::unstable_auto_guid()),
		R"(resources\imports\ktx\default_roughness.ktx)"
	);

	/**/

	delete(new(texture::DefaultAlpha));
	delete(new(texture::DefaultAO));
	delete(new(texture::DefaultDiffuse));
	delete(new(texture::DefaultMetalness));
	delete(new(texture::DefaultNormal));
	delete(new(texture::DefaultRoughness));

	/**/

	delete(new(material::DefaultBrdfMaterialPrototype));
	delete(new(material::DefaultBrdfMaterial));
}

static void initSkyboxDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createImageAsset(
		clone(image::DefaultSkybox::unstable_auto_guid()),
		R"(resources\imports\ktx\default_skybox.ktx)"
	);

	/**/

	delete(new(texture::DefaultSkybox));

	/**/

	delete(new(material::DefaultSkyboxPrototype));
	delete(new(material::DefaultSkybox));
}

static void initDirectoryIcons() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createImageAsset(
		clone(image::Directory::unstable_auto_guid()),
		R"(resources\imports\ktx\icons8-folder-144.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderAudio::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-audio.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderEnvironment::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-environment.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderFont::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-font.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderImages::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-images.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderImport::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-import.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderLog::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-log.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderResource::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-resource.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderShader::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-shader.ktx)"
	);

	factory->createImageAsset(
		clone(image::FolderVideo::unstable_auto_guid()),
		R"(resources\imports\ktx\folder-video.ktx)"
	);

	/**/

	delete(new(texture::Directory));
	delete(new(texture::FolderAudio));
	delete(new(texture::FolderEnvironment));
	delete(new(texture::FolderFont));
	delete(new(texture::FolderImages));
	delete(new(texture::FolderImport));
	delete(new(texture::FolderLog));
	delete(new(texture::FolderResource));
	delete(new(texture::FolderShader));
	delete(new(texture::FolderVideo));
}

static void initFileIcons() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createImageAsset(
		clone(image::Brand::unstable_auto_guid()),
		R"(resources\imports\ktx\brand.ktx)"
	);
}

static void initFontDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createFontAsset(
		clone(font::CascadiaCode::unstable_auto_guid()),
		R"(resources\imports\ttf\CascadiaCode.ttf)"

	);
}

static void initStaticGeometryDefaults() {

	using namespace ::hg::game::assets;
	const auto factory = Engine::getEngine()->getAssets()->getFactory();

	factory->createStaticGeometryAsset(
		clone(meshes::Sphere::unstable_auto_guid()),
		R"(resources\imports\obj\sphere.obj)",
		11520uLL,
		11520uLL

	);
}

#pragma endregion

void editor::boot::initAssets() {

	initMaterialDefaults();
	initSkyboxDefaults();
	initDirectoryIcons();
	initFileIcons();
	initFontDefaults();
	initStaticGeometryDefaults();

	/**/

	const auto root { std::filesystem::current_path().append(R"(resources\assets)") };
	if (not std::filesystem::exists(root)) {
		std::filesystem::create_directories(root);
	}

	Vector<path> backlog {};

	backlog.push_back(root);
	while (not backlog.empty()) {

		const auto cur { backlog.back() };
		backlog.pop_back();

		indexLoadable(cur, backlog);
	}

	/**/

	packageDiscoverAssets();

}

/**/

#include <Engine.Reflect/Reflect.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchive.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/SourceReadonlyArchive.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

void packageDiscoverAssets() {

	const auto* const pm = Engine::getEngine()->getResources()->packages(traits::nothrow);
	const auto view = pm->packages();

	for (const auto& resource : view) {

		const auto guard = resource->acquire(resource::ResourceUsageFlag::eRead);
		const auto package = guard.imm();
		const auto* const linker = package->getLinker();

		/* Check for stored archives */

		if (linker->getArchiveCount() <= 0) {
			continue;
		}

		/* Index / Load data from archives */

		const auto end = linker->cend();
		for (auto iter = linker->cbegin(); iter != end; ++iter) {

			if (iter.header().type != resource::ArchiveHeaderType::eSerializedStructure) {
				continue;
			}

			const auto linkedArchive = iter.archive();
			const serialization::StructuredArchive archive { linkedArchive.get() };

			/* Check for root stored assets */

			if (isArchivedAsset(archive.getRootSlot())) {

				linkedArchive->seek(0);
				tryLoadArchivedAsset(archive.getRootSlot());

				continue;
			}

			/* Check for a sequence of stored assets */

			{
				linkedArchive->seek(0);
				auto root = archive.getRootSlot();

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
