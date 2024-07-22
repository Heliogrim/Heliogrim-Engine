#include "WorldInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene/SceneFactory.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

static void addDefaultSkybox();

void editor::boot::initEditorWorld() {

	const auto engine { EditorEngine::getEngine() };
	const auto editor { engine->getEditorSession() };

	/**/

	auto scene = SceneFactory::createEditorScene();
	scene->prepare();

	const auto world { make_sptr<core::World>(std::move(scene)) };

	engine->addWorld(world);
	editor->getWorldContext().setCurrentWorld(world);
}

void editor::boot::initPrimaryWorld() {

	const auto engine { EditorEngine::getEngine() };
	const auto primary { engine->getPrimaryGameSession() };

	/**/

	auto scene = SceneFactory::createGameScene();
	scene->prepare();

	const auto world { make_sptr<core::World>(std::move(scene)) };

	engine->addWorld(world);
	primary->getWorldContext().setCurrentWorld(world);

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
#include <Heliogrim/World.hpp>

static void addDefaultSkybox() {

	auto session = GetSession();
	auto primaryWorld = GetWorld(session);

	auto* actor { CreateActor(session) };
	auto init { session.getActorInitializer() };
	auto sc = init.createComponent<SkyboxComponent>(actor);

	/**/

	auto query = Heliogrim::assets()[game::assets::material::DefaultSkybox::unstable_auto_guid()];
	sc->setSkyboxMaterialByAsset(static_cast<ref<GfxMaterialAsset>>(query.value));

	query = Heliogrim::assets()[editor::assets::meshes::default_sphere_guid];
	sc->setSkyboxGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

	/**/

	primaryWorld.addActor(actor);
}
