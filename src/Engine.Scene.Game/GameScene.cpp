#include "GameScene.hpp"

#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/ActorComponent.hpp>
#include <Heliogrim/SceneComponent.hpp>

using namespace hg::engine::scene;
using namespace hg;

void GameScene::add(const ptr<ActorComponent> component_) {
    if (auto* const sceneComponent = Cast<SceneComponent>(component_)) {
        Scene::add(sceneComponent);
    }
}

void GameScene::add(cref<ComponentHierarchy> hierarchy_) {
    std::unreachable();
}

nmpt<IComponentRegisterContext> GameScene::registerContext() noexcept {
    return this;
}
