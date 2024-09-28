#include "GameScene.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

using namespace hg::engine::scene;
using namespace hg;

void GameScene::add(const ptr<HierarchyComponent> component_) {
	if (auto* const sceneComponent = Cast<SceneComponent>(component_)) {
		Scene::add(sceneComponent);
	}
}

void GameScene::add(cref<ComponentHierarchy> hierarchy_) {
	::hg::todo_panic();
}

nmpt<IComponentRegisterContext> GameScene::registerContext() noexcept {
	return this;
}
