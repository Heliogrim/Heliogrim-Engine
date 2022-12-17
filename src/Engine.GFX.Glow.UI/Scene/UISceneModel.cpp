#include "UISceneModel.hpp"

#include <Engine.GFX/Cache/ModelBatch.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>

#include "Ember/UIComponent.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

UISceneModel::UISceneModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_) {}

void UISceneModel::create(const ptr<engine::scene::Scene> scene_) {}

void UISceneModel::update(const ptr<engine::scene::Scene> scene_) {}

void UISceneModel::destroy(const ptr<engine::scene::Scene> scene_) {}

Vector<engine::gfx::render::RenderDataToken> UISceneModel::providedToken() const noexcept {
    return {};
}

ptr<engine::gfx::cache::ModelBatch> UISceneModel::batch(const ptr<gfx::render::RenderPassState> state_) {
    return nullptr;
}

sptr<engine::reflow::Window> UISceneModel::getWindow() const noexcept {
    if (not _owner) {
        return nullptr;
    }
    return static_cast<ptr<UIComponent>>(_owner)->getWindow();
}
