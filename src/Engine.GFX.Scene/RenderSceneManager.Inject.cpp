#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX/Scene/CameraModel.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Scene/RevScene.hpp>

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

    /**/

    scene->setNodeType(
        tag,
        UIComponent::typeId,
        [](const ptr<SceneComponent> sc_) {
            return new reflow::UISceneModel(sc_);
        }
    );
}
