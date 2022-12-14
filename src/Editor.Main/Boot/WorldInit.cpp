#include "WorldInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Scene/SceneFactory.hpp>

#include "Engine.Core/WorldContext.hpp"

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

static void addDefaultSkybox();

void editor::boot::initEditorWorld() {

    auto* const engine { EditorEngine::getEngine() };
    const auto* const editor { engine->getEditorSession() };

    /**/

    auto scene { scene::SceneFactory::createDefaultScene() };
    const auto world { make_sptr<core::World>(_STD move(scene)) };

    engine->addWorld(world);
    editor->getWorldContext()->setCurrentWorld(world);
}

void editor::boot::initPrimaryWorld() {

    auto* const engine { EditorEngine::getEngine() };
    const auto* const primary { engine->getPrimaryGameSession() };

    /**/

    auto scene { scene::SceneFactory::createDefaultScene() };
    const auto world { make_sptr<core::World>(_STD move(scene)) };

    engine->addWorld(world);
    primary->getWorldContext()->setCurrentWorld(world);

    /**/

    addDefaultSkybox();
}

#include <Ember/Ember.hpp>
#include <Ember/Actor.hpp>
#include <Ember/ActorInitializer.hpp>
#include <Ember/SkyboxComponent.hpp>
#include <Ember/Session.hpp>
#include <Ember/World.hpp>

#include <Ember.Default/Assets/GfxMaterials/DefaultSkybox.hpp>
#include <Game.Main/Assets/Meshes/Sphere.hpp>

static void addDefaultSkybox() {

    auto session = GetSession();
    auto primaryWorld = GetWorld(session);

    auto* actor { CreateActor(session) };
    auto init { session.getActorInitializer() };
    auto sc = init.createComponent<SkyboxComponent>(actor);

    /**/

    auto query = Ember::assets()[game::assets::material::DefaultSkybox::unstable_auto_guid()];
    sc->setSkyboxMaterialByAsset(static_cast<ref<GfxMaterialAsset>>(query.value));

    query = Ember::assets()[game::assets::meshes::Sphere::unstable_auto_guid()];
    sc->setSkyboxGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

    /**/

    primaryWorld.addActor(actor);
}
