#include <Engine.Level/Level.hpp>

/**/

#include "UniverseInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
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

static Arci<core::Level> experimentalLoadLevel();

static void experimentalSaveLevel(ref<core::Level> level_);

/**/

void editor::boot::initEditorUniverse() {

	const auto engine { EditorEngine::getEngine() };
	const auto editorSession { engine->getEditorSession() };

	/**/

	auto scene = SceneFactory::createEditorScene();
	scene->prepare();

	auto level = core::make_root_like_level();
	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { clone(level) }) };
	engine->addUniverse(clone(universe));
	universe->commitLevel(*level.get());

	editorSession->getUniverseContext().setCurrentUniverse(std::move(universe));
}

void editor::boot::initPrimaryUniverse() {

	const auto engine { EditorEngine::getEngine() };
	const auto primary { engine->getPrimaryGameSession() };

	/**/

	auto level = experimentalHasLevel() ?
		experimentalLoadLevel() :
		([] {
			auto nextLevel = core::make_root_like_level();

			add_default_skybox(Level { clone(nextLevel) });
			add_default_light(Level { clone(nextLevel) });
			add_default_box(Level { clone(nextLevel) });
			experimentalSaveLevel(*nextLevel);

			return nextLevel;
		})();

	/**/

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { clone(level) }) };
	engine->addUniverse(clone(universe));
	universe->commitLevel(*level.get());

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

static decltype(auto) wantedLoad(_In_ cref<assets::LevelAsset> levelAsset_) {

	auto engine = Engine::getEngine();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

	/**/

	auto level = Arci<core::Level>::create(levelAsset_);
	ssys->query(
		std::move(dataArchive).value(),
		[&level_ = *level, serial](resource::StorageReadonlyArchive& archive_) -> void {
			auto structured = serialization::StructuredArchive { archive_ };
			serial->hydrate(structured, level_);
		}
	);

	return level;
}

static decltype(auto) wantedSave(_In_ cref<core::Level> level_, _In_ cref<assets::LevelAsset> levelAsset_) {

	auto engine = Engine::getEngine();
	const auto serial = engine->getSerialization();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<storage::ArchiveUrl>()));
	::hg::assertrt(dataArchive != None);

	ssys->mutate(
		std::move(dataArchive).value(),
		[&level_, serial](auto& archive_) {
			archive_.setType(resource::ArchiveType::eSerializedStructure);
			auto structured = serialization::StructuredArchive { archive_ };
			serial->serialize(level_, structured);
		}
	);

	/**/

	auto assetUrl = levelAsset_.getAssetStorageUrl();
	auto assetArchive = ssys->findArchive(clone(assetUrl.as<storage::ArchiveUrl>()));
	::hg::assertrt(assetArchive != None);

	ssys->mutate(
		::hg::move(assetArchive).value(),
		[&levelAsset_, serial](auto& archive_) {
			archive_.setType(resource::ArchiveType::eSerializedStructure);
			auto structured = serialization::StructuredArchive { archive_ };
			serial->serialize(levelAsset_, structured);
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

	/**/

	auto results = Vector<nmpt<assets::Asset>> {};
	assets->getRegistry()->findAssetsByPath(assets::AssetPath { "universe/default"sv }, { .recursive = true }, results);

	::hg::assertrt(not results.empty());

	const auto guesswork = std::ranges::find_if(
		results,
		[](const nmpt<assets::Asset> asset_) {
			return IsType<assets::LevelAsset>(*asset_) &&
				not static_cast<ref<const assets::LevelAsset>>(asset_.load()).getLevelData().valueless_by_exception();
		}
	);

	::hg::assertrt(guesswork != results.end());
	return (*guesswork)->arci_from_this<>().into<assets::LevelAsset>();
}

static Arci<assets::LevelAsset> experimentalResolveLevelAsset(ref<const engine::core::Level> level_) {

	const auto engine = Engine::getEngine();
	const auto assets = engine->getAssets();
	const auto ssys = engine->getStorage()->getSystem();

	/**/

	auto asset = assets->getRegistry()->findAssetByGuid(level_.assetGuid());
	::hg::assertrt(asset == None);

	/**/

	const auto levelDataArchiveGuid = resource::ArchiveGuid::random();
	const auto ensureDataArchive = ssys->requestArchive(
		storage::ArchiveUrl { levelDataArchiveGuid },
		{ .package = { .fileUrl = experimentalResolveLevelFileUrl() } }
	);
	::hg::assertrt(ensureDataArchive.has_value());

	const auto levelAssetArchiveGuid = resource::ArchiveGuid::random();
	const auto ensureAssetArchive = ssys->requestArchive(
		storage::ArchiveUrl { levelAssetArchiveGuid },
		{ .package = { .fileUrl = experimentalResolveLevelFileUrl() } }
	);
	::hg::assertrt(ensureAssetArchive.has_value());

	auto nextLevelAsset = Arci<assets::LevelAsset>::create(
		level_,
		"Root Level"sv,
		storage::ArchiveUrl { levelAssetArchiveGuid },
		experimentalResolveLevelAssetUrl()
	);
	nextLevelAsset->setLevelData(storage::ArchiveUrl { levelDataArchiveGuid });

	return nextLevelAsset;
}

bool experimentalHasLevel() {
	return Engine::getEngine()->getStorage()->getSystem()->hasStorage(experimentalResolveLevelFileUrl());
}

Arci<core::Level> experimentalLoadLevel() {
	const auto asset = experimentalResolveLevelAsset();
	return wantedLoad(*asset);
}

void experimentalSaveLevel(ref<core::Level> level_) {
	const auto asset = experimentalResolveLevelAsset(level_);
	wantedSave(level_, *asset);
}
