#include "RevScene.hpp"

#include <Ember/SceneComponent.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX/Scene/SceneNodeModel.hpp>

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
    _cached() {}

RevScene::~RevScene() = default;

void RevScene::update() {
    Scene::update();

    auto& graph { _renderGraph.asMutable() };

    /**
     * TODO: Rework
     *
     * Update each element in scene graph or fetch for destruction
     */
    Vector<ptr<gfx::SceneNodeModel>> markedForDestruction {};
    graph.traversal([scene = this, &markedForDestruction](auto* node_) {

        /**
         * Check for elements -> leaf node
         */
        if (node_->isLeaf()) {

            /**
             * Iterate over elements
             */
            const auto* const elements { node_->elements() };
            for (auto idx { node_->exclusiveSize() }; idx > 0; --idx) {

                /**
                 * Check for destruction marking, otherwise update element
                 */
                auto* const element { elements[idx - 1u] };
                if (element->markedAsDeleted()) {
                    markedForDestruction.push_back(element);

                } else {
                    element->update(scene);
                }

            }

        }

        return node_->inclusiveSize();
    });

    /**
     * Destroy every fetched marked node
     */
    for (auto&& marked : markedForDestruction) {

        [[maybe_unused]] const auto result { graph.pop(marked) };

        #ifdef _DEBUG
        assert(result);
        #endif

        EmberObject::destroy(_STD move(marked));
    }

    /**
     *
     */

    /**
     * Will swap `_cached` with a new vector
     */
    const auto list { _STD move(_cached) };
    for (auto&& entry : list) {

        /**
         * Precheck for already modified models 
         */
        if (entry.second->markedAsDeleted()) {
            // Warning: Unsafe !!
            delete entry.second;
            continue;
        }

        /**
         * Scene Mounting should be made by update of scene itself
         */
        entry.second->create(this);

        /**
         *
         */
        if (entry.first == gfx::GfxSceneTag {}) {
            /**
             *
             */
            graph.push(static_cast<const ptr<gfx::SceneNodeModel>>(entry.second));
        }
    }
}

const ember::ptr<RenderGraph> RevScene::renderGraph() noexcept {
    return &_renderGraph;
}

bool RevScene::addNode(const ptr<SceneComponent> node_) {
    return false;
}

bool RevScene::addNodeCached(const ptr<SceneComponent> node_) noexcept {

    /**
     * Warning: This might be the way we want to handle data between the modules
     *
     *  Every SceneComponent should be type indexed
     *  Every System should get the possibility to register callbacks per component type to create his own subobject
     *
     *  Actor -> StaticGeometryComponent
     *  :: pushed component to scene object
     *  :: type lookup  :: callback from graphics system
     *  the graphics system is in charge to setup his interpretation of the data so `graphics( component ) = graphics own proxy object`
     *
     *  *advanced*
     *  physics could also register a callback for the same component but might require another interpretation
     *  :: pushed component to object
     *  :: type lookup  :: callback for graphics and physics system
     *  `graphics( component ) = gfx proxy` + `physics( component ) = pfx proxy`
     */

    auto& ntre { _nodeTypeRegistry.get(node_->getTypeId()) };
    if (!ntre.gfx) {
        return false;
    }

    // TODO: Only create new model if entry does not already contain model type
    auto* model { ntre.gfx(node_) };

    /**
     * Push the model to the component usage set
     */
    node_->addSceneNodeModel(model);

    _cached.push_back(_STD make_pair(gfx::GfxSceneTag {}, model));
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
