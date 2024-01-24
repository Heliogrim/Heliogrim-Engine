#include "UISceneModel.hpp"

#include <Engine.GFX/Cache/ModelBatch.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/UIComponent.hpp>

using namespace hg::engine::reflow;
using namespace hg::engine::gfx;
using namespace hg;

UISceneModel::UISceneModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_) {}

void UISceneModel::create(const ptr<render::RenderSceneSystem> system_) {}

void UISceneModel::update(const ptr<render::RenderSceneSystem> system_) {}

void UISceneModel::destroy(const ptr<render::RenderSceneSystem> system_) {}

ref<Window> UISceneModel::getWindow() const noexcept {
    if (owner() == nullptr) {
        std::unreachable();
    }

    return *Cast<UIComponent>(owner())->getWindow();
}
