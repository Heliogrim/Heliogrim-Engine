#include <Engine.Level/Level.hpp>

/**/

#include "UniverseInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Heliogrim/Universe/Level.hpp>

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

/**/

static void add_default_skybox(mref<Level> level_);

static void add_default_light(mref<Level> level_);

static void add_default_box(mref<Level> level_);

static bool experimentalHasLevel();

static void experimentalLoadLevel(ref<core::Level> level_);

static void experimentalSaveLevel(ref<core::Level> level_);

/**/

void editor::boot::initEditorUniverse() {

	const auto engine { EditorEngine::getEngine() };
	const auto editorSession { engine->getEditorSession() };

	/**/

	auto scene = SceneFactory::createEditorScene();
	scene->prepare();

	auto level = core::make_root_like_level();
	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { std::move(level) }) };

	engine->addUniverse(clone(universe));
	editorSession->getUniverseContext().setCurrentUniverse(std::move(universe));
}

void editor::boot::initPrimaryUniverse() {

	const auto engine { EditorEngine::getEngine() };
	const auto primary { engine->getPrimaryGameSession() };

	/**/

	auto level = core::make_root_like_level();
	const bool levelData = experimentalHasLevel();

	if (not levelData) {
		add_default_skybox(Level { clone(level) });
		add_default_light(Level { clone(level) });
		add_default_box(Level { clone(level) });
		experimentalSaveLevel(*level);

	} else {
		experimentalLoadLevel(*level);
	}

	/**/

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { clone(level) }) };
	engine->addUniverse(clone(universe));

	auto* const keepRef = level.get();
	universe->addLevel(::hg::move(level));
	universe->commitLevel(keepRef);

	primary->getUniverseContext().setCurrentUniverse(std::move(universe));
}

/**/

#include <Editor.Assets.Default/GfxMaterials/Default.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Asset/GeometryAssetHandles.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

static void add_default_skybox(mref<Level> level_) {

	const auto meshQuery = Heliogrim::assets().find<StaticGeometryAssetHandle>(editor::assets::meshes::default_sphere_guid);
	::hg::assertrt(static_cast<bool>(meshQuery));

	const auto materialQuery = Heliogrim::assets().find<GfxMaterialAssetHandle>(editor::assets::materials::default_skybox_guid);
	::hg::assertrt(static_cast<bool>(materialQuery));

	auto actor = CreateActor();
	actor->addComponent<SkyboxComponent>({ .parent = None }, clone(meshQuery.value), clone(materialQuery.value));

	#if 1
	[[maybe_unused]] auto skybox = actor->addComponent<SkyboxComponent>();
	auto box2 = actor->addComponent<StaticGeometryComponent>(StaticGeometryAssetHandle {}, AutoArray<ptr<const GfxMaterialAssetHandle>> {});
	auto parent2 = box2.map_or(
		[](auto& obj_) -> Opt<ref<HierarchyComponent>> { return Some<ref<HierarchyComponent>>(obj_); },
		Opt<ref<HierarchyComponent>> {}
	);
	[[maybe_unused]] auto skybox3 = actor->addComponent<SkyboxComponent>({ .parent = parent2 });
	auto box4 = actor->addComponent<StaticGeometryComponent>(
		{ .parent = parent2 },
		StaticGeometryAssetHandle {},
		AutoArray<ptr<const GfxMaterialAssetHandle>> {}
	);

	actor->destroyComponent<>(*box4);
	actor->destroyComponent<>(*box2);
	::hg::assertd(not actor->getComponents().contains(std::addressof(*skybox3)));
	#endif

	/**/

	level_.addActor(::hg::move(actor));
}

static void add_default_light(mref<Level> level_) {

	auto actor = CreateActor();
	auto component = actor->addComponent<DirectionalLightComponent>();

	/**/

	component->_luminance = { 0.74f, 0.67f, 0.71f };
	component->_direction = { 0.26f, 0.5236f, 0.0f };

	/**/

	level_.addActor(::hg::move(actor));
}

static void add_default_box(mref<Level> level_) {

	const auto meshQuery = Heliogrim::assets().find<StaticGeometryAssetHandle>(editor::assets::meshes::default_cube_guid);
	::hg::assertrt(static_cast<bool>(meshQuery));

	const auto materialQuery = Heliogrim::assets().find<GfxMaterialAssetHandle>(editor::assets::materials::default_brdf_guid);
	::hg::assertrt(static_cast<bool>(materialQuery));

	auto actor = CreateActor();
	actor->addComponent<StaticGeometryComponent>(
		clone(meshQuery.value),
		AutoArray<ptr<const GfxMaterialAssetHandle>> { &materialQuery.value }
	);

	/**/

	level_.addActor(::hg::move(actor));
}

/**/

static decltype(auto) wantedLoad(_In_ cref<assets::LevelAsset> levelAsset_, _Inout_ ref<core::Level> level_) {

	auto engine = Engine::getEngine();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

	/**/

	ssys->query(
		std::move(dataArchive).value(),
		[&level_, serial](resource::StorageReadonlyArchive& archive_) -> void {
			auto structured = serialization::StructuredArchive { archive_ };
			serial->hydrate(structured, level_);
		}
	);
}

static decltype(auto) wantedSave(_In_ cref<core::Level> level_, _In_ cref<assets::LevelAsset> levelAsset_) {

	auto engine = Engine::getEngine();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

	/**/

	ssys->mutate(
		std::move(dataArchive).value(),
		[&level_, serial](auto& archive_) {
			archive_.setType(resource::ArchiveType::eSerializedStructure);
			auto structured = serialization::StructuredArchive { archive_ };
			serial->serialize(level_, structured);
		}
	);
}

static assets::AssetUrl experimentalResolveLevelAssetUrl() {
	return assets::AssetUrl {
		"universe/default"sv,
		"Root Level"sv
	};
}

static storage::FileUrl experimentalResolveLevelFileUrl() {

	const auto engine = Engine::getEngine();
	const auto& config = engine->getConfig();

	/**/

	// Note: We may not need the project root, as the storage engine will resolve the relative path based on the setup configuration by itself.
	auto projectRoot = config.getTyped<String>(cfg::ProjectConfigProperty::eLocalAssetPath);
	::hg::assertrt(projectRoot.has_value());

	constexpr auto subpath = "/universe/default/"sv;
	constexpr auto filename = "root.level.imp";

	return storage::FileUrl {
		clone(storage::FileProjectScheme),
		engine::fs::Path { projectRoot->value().append(subpath).append(filename) }
	};
}

static Arci<assets::LevelAsset> experimentalResolveLevelAsset() {

	const auto engine = Engine::getEngine();
	const auto assets = engine->getAssets();
	const auto ssys = engine->getStorage()->getSystem();

	/**/

	constexpr auto levelAssetTypeId = "::experimental::assets::RootLevel"_typeId;
	constexpr auto levelAssetGuid = Guid {
		static_cast<u32>(levelAssetTypeId.data >> 32),
		static_cast<u16>(levelAssetTypeId.data >> 16),
		static_cast<u16>(levelAssetTypeId.data >> 8),
		levelAssetTypeId.data
	};

	constexpr auto levelTypeId = "::experimental::RootLevel"_typeId;
	constexpr auto levelArchiveGuid = Guid {
		static_cast<u32>(levelTypeId.data >> 32),
		static_cast<u16>(levelTypeId.data >> 16),
		static_cast<u16>(levelTypeId.data >> 8),
		levelTypeId.data
	};

	/**/

	auto asset = assets->getRegistry()->findAssetByGuid(levelAssetGuid);
	if (asset != None && (*asset)->getTypeId() == assets::LevelAsset::typeId) {
		return std::move(asset).value().into<assets::LevelAsset>();
	}

	/**/

	const auto ensure = ssys->requestArchive(
		storage::ArchiveUrl { resource::ArchiveGuid::from(levelArchiveGuid) },
		{ .package = { .fileUrl = experimentalResolveLevelFileUrl() } }
	);
	::hg::assertrt(ensure.has_value());

	auto nextLevelAsset = Arci<assets::LevelAsset>::create(
		generate_asset_guid(),
		"Root Level"sv,
		assets::AssetReferenceUrl {},
		experimentalResolveLevelAssetUrl()
	);
	nextLevelAsset->setLevelData(storage::ArchiveUrl { resource::ArchiveGuid::from(levelArchiveGuid) });

	return nextLevelAsset;
}

bool experimentalHasLevel() {
	return Engine::getEngine()->getStorage()->getSystem()->hasStorage(experimentalResolveLevelFileUrl());
}

void experimentalLoadLevel(ref<core::Level> level_) {
	const auto asset = experimentalResolveLevelAsset();
	wantedLoad(*asset, level_);
}

void experimentalSaveLevel(ref<core::Level> level_) {
	const auto asset = experimentalResolveLevelAsset();
	wantedSave(level_, *asset);
}
