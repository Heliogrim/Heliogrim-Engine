#include "PreviewScene.hpp"

#include <Engine.ACS/Component/HierarchyComponent.hpp>
#include <Engine.ACS/Component/SceneComponent.hpp>

import Engine.Asserts;
import Engine.Reflect;

using namespace hg::engine::scene;
using namespace hg;

void PreviewScene::add(const ptr<HierarchyComponent> component_) {
	if (auto* const sceneComponent = Cast<SceneComponent>(component_)) {
		Scene::add(sceneComponent);
	}
}

void PreviewScene::add(cref<ComponentHierarchy> hierarchy_) {
	::hg::todo_panic();
}

void PreviewScene::remove(ptr<HierarchyComponent> component_) {
	if (auto* const sceneComponent = Cast<SceneComponent>(component_)) {
		Scene::remove(sceneComponent);
	}
}

void PreviewScene::remove(ref<const ComponentHierarchy> hierarchy_) {
	::hg::todo_panic();
}

nmpt<IComponentRegisterContext> PreviewScene::registerContext() noexcept {
	return this;
}