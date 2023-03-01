#include <Heliogrim/SkyboxComponent.hpp>
#include <Heliogrim/StaticGeometryComponent.hpp>
#include <Heliogrim/UIComponent.hpp>
#include <Heliogrim/Components/CameraComponent.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX/Scene/StaticGeometryModel.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Glow.UI/Scene/UISceneModel.hpp>
#include <Engine.GFX/Scene/CameraModel.hpp>
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
        HeliogrimObject::create<CameraModel, const ptr<SceneComponent>>
    );

    /**/

    scene->setNodeType(
        tag,
        StaticGeometryComponent::typeId,
        HeliogrimObject::create<StaticGeometryModel, const ptr<SceneComponent>>
    );
    scene->setNodeType(
        tag,
        SkyboxComponent::typeId,
        HeliogrimObject::create<SkyboxModel, const ptr<SceneComponent>>
    );

    /**/

    scene->setNodeType(
        tag,
        UIComponent::typeId,
        HeliogrimObject::create<glow::ui::UISceneModel, const ptr<SceneComponent>>
    );
}
