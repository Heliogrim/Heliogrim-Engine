#include "RevScene.hpp"

#include <Ember/SceneComponent.hpp>
#include <Ember/World.hpp>

using namespace ember::engine::scene;

RenderGraph makeRenderGraph() {

    auto nodeStorage { ember::make_sptr<RenderGraph::node_storage_type>() };
    auto elementStorage { ember::make_sptr<RenderGraph::element_storage_type>() };

    auto root {
        SceneNodeFactory<RenderGraph::node_storage_type, RenderGraph::element_storage_type, RenderGraph::traits> {
            nodeStorage.get(),
            elementStorage.get()
        }.assembleRoot()
    };

    return {
        _STD move(root.first),
        _STD move(nodeStorage),
        _STD move(elementStorage)
    };
}

RevScene::RevScene() noexcept :
    Scene(),
    _renderGraph(makeRenderGraph()),
    _cachedNew(),
    _world(nullptr) {

    _world = make_uptr<World>(this);
}

RevScene::~RevScene() = default;

void RevScene::update() {
    Scene::update();

    auto& graph { _renderGraph.asMutable() };

    /**
     * Will swap _cachedNew with a new vector
     */
    const auto list { _STD move(_cachedNew) };
    for (auto& entry : list) {
        //graph.push(entry);

        /**
         *  Warning: This might be the way we want to handle data between the modules
         *
         *  Every SceneComponent should be typ indexed
         *  Every System should get the possibility to register callbacks per component type to create his own subobject
         *
         *  Actor -> StaticGeometryComponent
         *  :: pushed component to scene object
         *  :: type loopup  :: callback from graphics system
         *  the graphics system is in charge to setup his interpretation of the data so `graphics( component ) = graphics own proxy object`
         *
         *  *advanced*
         *  physics could also register a callback for the same component but might require another interpretation
         *  :: pushed component to object
         *  :: type lookup  :: callback for graphics and physics system
         *  `graphics( component ) = gfx proxy` + `physics( component ) = pfx proxy`
         */

        auto& ntre { _nodeTypeRegistry.get(entry->typeId()) };
        if (!ntre.gfx) {
            continue;
        }

        // TODO: Only create new model if entry does not already contain model type
        auto* model { ntre.gfx(entry) };
        model->create(this);
    }
}

const ember::ptr<RenderGraph> RevScene::renderGraph() noexcept {
    return &_renderGraph;
}

bool RevScene::addNode(const ptr<SceneComponent> node_) {
    return false;
}

bool RevScene::addNodeCached(const ptr<SceneComponent> node_) noexcept {
    _cachedNew.push_back(node_);
    return true;
}

bool RevScene::removeNode(const ptr<SceneComponent> node_) {
    return false;
}

bool RevScene::removeNodeCached(const ptr<SceneComponent> node_) noexcept {
    return false;
}

ember::ptr<ember::IComponentRegisterContext> RevScene::registerContext() noexcept {
    return static_cast<ember::ptr<ember::IComponentRegisterContext>>(this);
}

void RevScene::add(const ptr<ActorComponent> component_) {
    // Warning: Unsafe
    addNodeCached(static_cast<const ptr<SceneComponent>>(component_));
}

void RevScene::add(cref<ComponentHierarchy> hierarchy_) {
    throw NotImplementedException {};
}

ember::ptr<ember::World> RevScene::getWorld() noexcept {
    return _world.get();
}
