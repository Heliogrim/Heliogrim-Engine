#include "UISceneModel.hpp"

#include <Engine.GFX/Cache/ModelBatch.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>

using namespace ember::engine::reflow;
using namespace ember::engine::gfx;
using namespace ember;

UISceneModel::UISceneModel(const ptr<SceneComponent> owner_) :
    SceneNodeModel(owner_) {}

void UISceneModel::create(const ptr<scene::Scene> scene_) {}

void UISceneModel::update(const ptr<scene::Scene> scene_) {}

void UISceneModel::destroy(const ptr<scene::Scene> scene_) {}

Vector<render::RenderDataToken> UISceneModel::providedToken() const noexcept {
    return {};
}

ptr<cache::ModelBatch> UISceneModel::batch(const ptr<render::RenderPassState> state_) {
    return nullptr;
}
