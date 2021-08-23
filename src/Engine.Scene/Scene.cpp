#include "Scene.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "./Node/SceneNodeFactory.hpp"

using namespace ember::engine::scene;
using namespace ember;

ptr<Scene> Scene::_instance = nullptr;

sptr<EmberSceneNodeStorage> tmp_build_storage() {
    return make_sptr<EmberSceneNodeStorage>();
}

Scene::Scene() noexcept = default;

Scene::~Scene() noexcept = default;

void Scene::setupGfxScene() {
    auto storage = make_sptr<EmberSceneNodeStorage>();
    auto factory = SceneNodeFactory(storage);

    auto result = factory.assembleRoot();

    auto graph = make_uptr<SceneGraph>(result.head, storage);

    _graphs.push_back(_STD move(graph));
}

void Scene::setupPfxScene() {
    auto storage = make_sptr<EmberSceneNodeStorage>();
    auto factory = SceneNodeFactory(storage);

    auto result = factory.assembleRoot();

    auto graph = make_uptr<SceneGraph>(result.head, storage);

    _graphs.push_back(_STD move(graph));
}

void Scene::setupSfxScene() {
    auto storage = make_sptr<EmberSceneNodeStorage>();
    auto factory = SceneNodeFactory(storage);

    auto result = factory.assembleRoot();

    auto graph = make_uptr<SceneGraph>(result.head, storage);

    _graphs.push_back(_STD move(graph));
}

void Scene::setup() {
    setupGfxScene();
    // setupPfxScene();
    // setupSfxScene();
}

ptr<Scene> Scene::get() noexcept {
    return _instance;
}

ptr<Scene> Scene::make() {

    if (!_instance) {
        _instance = new Scene();
    }

    return _instance;
}

void Scene::destroy() noexcept {
    delete _instance;
    _instance = nullptr;
}

cref<vector<uptr<SceneGraph>>> Scene::graphs() const noexcept {
    return _graphs;
}

ref<vector<uptr<SceneGraph>>> Scene::graphs() noexcept {
    return _graphs;
}
