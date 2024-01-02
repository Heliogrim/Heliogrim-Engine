#include <Engine.GFX/Scene/CameraModel.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.GFX.Scene.Model/PointLightModel.hpp>
#include <Engine.GFX.Scene.Model/SpotLightModel.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Heliogrim/Components/DirectionalLightComponent.hpp>
#include <Heliogrim/Components/PointLightComponent.hpp>
#include <Heliogrim/Components/SpotLightComponent.hpp>

#include "RenderSceneManager.hpp"

using namespace hg::engine::gfx::scene;
using namespace hg::engine::gfx;
using namespace hg;

void RenderSceneManager::injectSceneHooks(const ptr<RenderScene> renderScene_) {

    auto* const scene { static_cast<const ptr<::hg::engine::scene::RevScene>>(renderScene_) };
    constexpr GfxSceneTag tag {};

    /**/

    scene->setNodeType(
        tag,
        CameraComponent::typeId,
        [](const ptr<SceneComponent> sc_) {
            return new CameraModel(sc_);
        }
    );

    /**/

    scene->setNodeType(
        tag,
        StaticGeometryComponent::typeId,
        [](const ptr<SceneComponent> sc_) {
            return new StaticGeometryModel(sc_);
        }
    );
    scene->setNodeType(
        tag,
        SkyboxComponent::typeId,
        [](const ptr<SceneComponent> sc_) {
            return new SkyboxModel(sc_);
        }
    );
    scene->setNodeType(
        tag,
        PointLightComponent::typeId,
        [](const ptr<SceneComponent> obj_) {
            return new PointLightModel(obj_);
        }
    );
    scene->setNodeType(
        tag,
        DirectionalLightComponent::typeId,
        [](const ptr<SceneComponent> obj_) {
            return new DirectionalLightModel(obj_);
        }
    );
    scene->setNodeType(
        tag,
        SpotLightComponent::typeId,
        [](const ptr<SceneComponent> obj_) {
            return new SpotLightModel(obj_);
        }
    );

    /**/

    scene->setNodeType(
        tag,
        UIComponent::typeId,
        [](const ptr<SceneComponent> sc_) {
            return new reflow::UISceneModel(sc_);
        }
    );
}
