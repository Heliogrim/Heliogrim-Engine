#include "UISceneModel.hpp"

#include <Engine.Graphics.Loader/Cache/ModelBatch.hpp>
#include <Engine.Reflow.Components/UI/UIComponent.hpp>

import Engine.Asserts;
import Engine.Reflect;

using namespace hg::engine::reflow;
using namespace hg::engine::graphics;
using namespace hg;

UISceneModel::UISceneModel(const ptr<SceneComponent> owner_) :
	InheritMeta(owner_) {}

void UISceneModel::create(const ptr<render::RenderSceneSystem> system_) {}

void UISceneModel::update(const ptr<render::RenderSceneSystem> system_) {}

void UISceneModel::destroy(const ptr<render::RenderSceneSystem> system_) {}

ref<Window> UISceneModel::getWindow() const noexcept {
	::hg::assertrt(owner() != nullptr);
	return *Cast<UIComponent>(owner())->getWindow();
}
