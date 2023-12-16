#include "WorldInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.Scene/SceneFactory.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Heliogrim.Default/Assets/GfxMaterials/DefaultBrdfMaterial.hpp>

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

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

#include <Game.Main/Assets/Meshes/Sphere.hpp>
#include <Heliogrim/Actor.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/Session.hpp>
#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/World.hpp>
#include <Heliogrim.Default/Assets/GfxMaterials/DefaultSkybox.hpp>

static void addDefaultSkybox() {

    auto session = GetSession();
    auto primaryWorld = GetWorld(session);

    auto* actor { CreateActor(session) };
    auto init { session.getActorInitializer() };
    auto sc = init.createComponent<SkyboxComponent>(actor);

    /**/

    auto query = Heliogrim::assets()[game::assets::material::DefaultSkybox::unstable_auto_guid()];
    sc->setSkyboxMaterialByAsset(static_cast<ref<GfxMaterialAsset>>(query.value));

    query = Heliogrim::assets()[game::assets::meshes::Sphere::unstable_auto_guid()];
    sc->setSkyboxGeometryByAsset(static_cast<ref<StaticGeometryAsset>>(query.value));

    /**/

    primaryWorld.addActor(actor);

    /* Warning: Test Code */

    for (u32 i = 0; i < 5; ++i) {
        auto* actor = CreateActor(session);
        auto init = session.getActorInitializer();
        auto* sma = init.createComponent<StaticGeometryComponent>(actor);

        auto query = Heliogrim::assets()[game::assets::material::DefaultBrdfMaterial::unstable_auto_guid()];
        const_cast<Vector<GfxMaterialAsset>&>(sma->overrideMaterials()).emplace_back(
            static_cast<cref<GfxMaterialAsset>>(query.value)
        );

        query = Heliogrim::assets()[game::assets::meshes::Sphere::unstable_auto_guid()];
        sma->setStaticGeometryByAsset(static_cast<cref<StaticGeometryAsset>>(query.value));

        /**/

        {
            cref<math::Transform> tf = actor->getRootComponent()->getWorldTransform();

            switch (i) {
                case 0: {
                    const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 3.F, 0.F, 3.F });
                    break;
                }
                case 1: {
                    const_cast<ref<math::Transform>>(tf).setLocation(math::Location { -3.F, 0.F, 3.F });
                    break;
                }
                case 2: {
                    const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 3.F, 0.F, -3.F });
                    break;
                }
                case 3: {
                    const_cast<ref<math::Transform>>(tf).setLocation(math::Location { -3.F, 0.F, -3.F });
                    break;
                }
                case 4: {
                    const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 0.F, 0.F, 0.F });
                    break;
                }
                default: {}
            }
        }

        /**/

        primaryWorld.addActor(actor);
    }
}
