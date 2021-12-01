#include "Scene.hpp"

using namespace ember::engine::scene;
using namespace ember;

Scene::Scene() noexcept = default;

Scene::~Scene() = default;

void Scene::update() {}

ptr<World> Scene::getWorld() {
    return nullptr;
}
