#include "Scene.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "./Node/SceneNodeFactory.hpp"

using namespace ember::engine::scene;
using namespace ember;

ptr<Scene> Scene::_instance = nullptr;

Scene::Scene() noexcept = default;

Scene::~Scene() noexcept = default;

void Scene::setup() {
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

ptr<SceneGraph> Scene::getGraph(cref<SceneGraphTagBase> tag_) noexcept {

    const auto entry = _STD find_if(_taggedGraphs.begin(), _taggedGraphs.end(), [&tag_](const auto& entry) {
        return entry.first == tag_;
    });

    if (entry != _taggedGraphs.end()) {
        return entry->second;
    }

    return nullptr;
}

ptr<SceneGraph> Scene::getOrCreateGraph(cref<SceneGraphTagBase> tag_) {

    auto result = getGraph(tag_);

    if (result == nullptr) {

        auto storage = make_sptr<EmberSceneNodeStorage>();

        const auto factory = SceneNodeFactory(storage);
        auto root = factory.assembleRoot();

        result = _graphs.insert(
            _graphs.cend(),
            _STD move(make_uptr<SceneGraph>(root.head, storage))
        )->get();

        _taggedGraphs.push_back({ tag_, result });
    }

    return result;
}
