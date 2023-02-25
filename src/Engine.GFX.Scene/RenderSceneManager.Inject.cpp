#include <Ember/SkyboxComponent.hpp>
#include <Ember/StaticGeometryComponent.hpp>
#include <Ember/UIComponent.hpp>
#include <Ember/Components/CameraComponent.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX/Scene/StaticGeometryModel.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Glow.UI/Scene/UISceneModel.hpp>
#include <Engine.GFX/Scene/CameraModel.hpp>
#include <Engine.Scene/RevScene.hpp>

#include "RenderSceneManager.hpp"

using namespace ember::engine::gfx::scene;
using namespace ember::engine::gfx;
using namespace ember;

void RenderSceneManager::injectSceneHooks(const ptr<RenderScene> renderScene_) {

    auto* const scene { static_cast<const ptr<::ember::engine::scene::RevScene>>(renderScene_) };
    constexpr GfxSceneTag tag {};

    /**/

    scene->setNodeType(
        tag,
        CameraComponent::typeId,
        EmberObject::create<CameraModel, const ptr<SceneComponent>>
    );

    /**/

    scene->setNodeType(
        tag,
        StaticGeometryComponent::typeId,
        EmberObject::create<StaticGeometryModel, const ptr<SceneComponent>>
    );
    scene->setNodeType(
        tag,
        SkyboxComponent::typeId,
        EmberObject::create<SkyboxModel, const ptr<SceneComponent>>
    );

    /**/

    scene->setNodeType(
        tag,
        UIComponent::typeId,
        EmberObject::create<glow::ui::UISceneModel, const ptr<SceneComponent>>
    );
}
