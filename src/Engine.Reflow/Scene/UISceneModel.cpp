#include "UISceneModel.hpp"

#include <Engine.GFX/Cache/ModelBatch.hpp>
#include <Engine.GFX.Renderer/RenderPassState.hpp>
#include <Engine.GFX.Renderer/RenderDataToken.hpp>

using namespace hg::engine::reflow;
using namespace hg::engine::gfx;
using namespace hg;

UISceneModel::UISceneModel(const ptr<SceneComponent> owner_) :
    InheritMeta(owner_) {}

void UISceneModel::create(const ptr<scene::Scene> scene_) {}

void UISceneModel::update(const ptr<scene::Scene> scene_) {}

void UISceneModel::destroy(const ptr<scene::Scene> scene_) {}

Vector<render::RenderDataToken> UISceneModel::providedToken() const noexcept {
    return {};
}

ptr<cache::ModelBatch> UISceneModel::batch(const ptr<render::RenderPassState> state_) {
    return nullptr;
}
