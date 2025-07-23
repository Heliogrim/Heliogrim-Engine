#include <Engine.Level/Level.hpp>

/**/

#include "UniverseInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
#include <Engine.ACS/ActorModule.hpp>
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
	add_default_skybox(Level { clone(level) });

	/**/

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { clone(level) }) };
	engine->addUniverse(clone(universe));

	universe->commitLevel(std::move(level));
	primary->getUniverseContext().setCurrentUniverse(std::move(universe));
}

/**/

#include <Editor.Assets.Default/GfxMaterials/Default.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Asset/GeometryAssetHandles.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

static void add_default_skybox(mref<Level> level_) {

	auto actor { CreateActor() };
	auto init = ActorInitializer { *Engine::getEngine()->getActors()->getRegistry() };
	auto sc = init.createComponent<SkyboxComponent>(actor.get());

	/**/

	const auto material = Heliogrim::assets().find<GfxMaterialAssetHandle>(editor::assets::materials::default_skybox_guid);
	::hg::assertrt(static_cast<bool>(material));
	sc->setSkyboxMaterial(material.value);

	const auto meshQuery = Heliogrim::assets().find<StaticGeometryAssetHandle>(editor::assets::meshes::default_sphere_guid);
	::hg::assertrt(static_cast<bool>(meshQuery));
	sc->setSkyboxGeometry(meshQuery.value);

	/**/

	level_.addActor(::hg::move(actor));
}
