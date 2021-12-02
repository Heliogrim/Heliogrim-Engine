#include "RevScene.hpp"

#include <Ember/World.hpp>
#include "Graph/MutableSceneGraph.hpp"
#include "Node/SceneNodeFactory.hpp"

using namespace ember::engine::scene;

SceneGraph makeSceneGraph() {
    auto storage { ember::make_sptr<EmberSceneNodeStorage>() };
    auto root { SceneNodeFactory { storage }.assembleRoot() };

    return { _STD move(root.head), _STD move(storage) };
}

RevScene::RevScene() noexcept :
    Scene(),
    _graph(makeSceneGraph()),
    _cachedNew(),
    _world(nullptr) {

    _world = make_uptr<World>(this);
}

RevScene::~RevScene() = default;

void RevScene::update() {
    Scene::update();

    auto& graph { _graph.asMutable() };

    const auto list { _STD move(_cachedNew) };
    /*
    for (auto entry : list) {
        graph.push({
            nullptr,
            entry->getWorldTransform(),
            entry->getBoundaries()
        });
    }
     */
}

const ember::ptr<SceneGraph> RevScene::renderGraph() noexcept {
    return &_graph;
}

bool RevScene::addNode(const ptr<ActorComponent> node_) {
    return false;
}

bool RevScene::addNodeCached(const ptr<ActorComponent> node_) noexcept {
    return false;
}

bool RevScene::removeNode(const ptr<ActorComponent> node_) {
    return false;
}

bool RevScene::removeNodeCached(const ptr<ActorComponent> node_) noexcept {
    return false;
}

ember::ptr<ember::IComponentRegisterContext> RevScene::registerContext() noexcept {
    return static_cast<ember::ptr<ember::IComponentRegisterContext>>(this);
}

void RevScene::add(const ptr<ActorComponent> component_) {
    addNodeCached(component_);
}

void RevScene::add(cref<ComponentHierarchy> hierarchy_) {
    throw NotImplementedException {};
}

ember::ptr<ember::World> RevScene::getWorld() noexcept {
    return _world.get();
}
