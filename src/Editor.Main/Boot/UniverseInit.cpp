#include "UniverseInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
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

	const auto universe { make_sptr<core::Universe>(std::move(scene)) };

	engine->addUniverse(universe);
	editorSession->getUniverseContext().setCurrentUniverse(universe);
}

void editor::boot::initPrimaryUniverse() {

	const auto engine { EditorEngine::getEngine() };
	const auto primary { engine->getPrimaryGameSession() };

	/**/

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	const auto universe { make_sptr<core::Universe>(std::move(scene)) };

	engine->addUniverse(universe);
	primary->getUniverseContext().setCurrentUniverse(universe);

	/**/

	addDefaultSkybox();
}

#include <Editor.Assets.Default/GfxMaterials/DefaultSkybox.hpp>
#include <Editor.Assets.Default/Meshes/Default.hpp>
#include <Heliogrim/Actor.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Session.hpp>
#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/Universe.hpp>

static void addDefaultSkybox() {

	auto session = GetSession();
	auto primaryUniverse = GetUniverse(session);

	auto* actor { CreateActor(session) };
	auto init { session.getActorInitializer() };
	auto sc = init.createComponent<SkyboxComponent>(actor);

	/**/

	auto query = Heliogrim::assets()[game::assets::material::DefaultSkybox::unstable_auto_guid()];
	sc->setSkyboxMaterialByAsset(static_cast<ref<GfxMaterialAsset>>(query.value));

	query = Heliogrim::assets()[editor::assets::meshes::default_sphere_guid];
	sc->setSkyboxGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

	/**/

	primaryUniverse.addActor(actor);
}
