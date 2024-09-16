#include <Engine.Level/Level.hpp>

/**/

#include "UniverseInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

static void addDefaultSkybox();

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

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	auto level = core::make_root_like_level();
	auto universe { make_sptr<core::Universe>(std::move(scene), DenseSet<Arci<core::Level>> { clone(level) }) };

	engine->addUniverse(clone(universe));
	primary->getUniverseContext().setCurrentUniverse(std::move(universe));

	/**/

	addDefaultSkybox();
}

#include <Editor.Assets.Default/GfxMaterials/DefaultSkybox.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>
#include <Heliogrim/Core/Session.hpp>
#include <Heliogrim/Universe/Universe.hpp>

static void addDefaultSkybox() {

	auto session = GetSession();
	auto primaryUniverse = GetUniverse(session);

	auto actor { CreateActor() };
	auto init = ActorInitializer { *Engine::getEngine()->getActors()->getRegistry() };
	auto sc = init.createComponent<SkyboxComponent>(actor.get());

	/**/

	auto query = Heliogrim::assets()[game::assets::material::DefaultSkybox::unstable_auto_guid()];
	sc->setSkyboxMaterialByAsset(static_cast<ref<GfxMaterialAsset>>(query.value));

	query = Heliogrim::assets()[editor::assets::meshes::default_sphere_guid];
	sc->setSkyboxGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

	/**/

	primaryUniverse.addActor(std::move(actor));
}
