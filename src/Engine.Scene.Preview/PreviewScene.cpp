#include "PreviewScene.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/ActorComponent.hpp>
#include <Heliogrim/SceneComponent.hpp>

using namespace hg::engine::scene;
using namespace hg;

void PreviewScene::add(const ptr<ActorComponent> component_) {
	if (auto* const sceneComponent = Cast<SceneComponent>(component_)) {
		Scene::add(sceneComponent);
	}
}

void PreviewScene::add(cref<ComponentHierarchy> hierarchy_) {
	::hg::todo_panic();
}

nmpt<IComponentRegisterContext> PreviewScene::registerContext() noexcept {
	return this;
}
