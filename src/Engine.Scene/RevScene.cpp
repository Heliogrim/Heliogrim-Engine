#include "RevScene.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.GFX/Scene/SceneTag.hpp>
#include <Engine.GFX.Scene.Model/SceneModel.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Heliogrim/SceneComponent.hpp>

using namespace hg::engine::scene;

RenderSceneGraph makeRenderGraph() {

    auto nodeStorage { hg::make_sptr<RenderSceneGraph::node_storage_type>() };
    auto elementStorage { hg::make_sptr<RenderSceneGraph::element_storage_type>() };

    auto root {
        SceneNodeFactory<RenderSceneGraph::node_storage_type, RenderSceneGraph::element_storage_type,
            RenderSceneGraph::traits> {
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
    InheritMeta(),
    _renderGraph(makeRenderGraph()),
    _resourcePool(make_uptr<gfx::SceneResourcePool>(Engine::getEngine()->getGraphics()->getCurrentDevice())),
    _cached() {
    _resourcePool->setup();
}

RevScene::~RevScene() {
    _resourcePool->destroy();
}

void RevScene::update() {
    Scene::update();

    auto& graph { _renderGraph.asMutable() };

    /**
     * TODO: Rework
     *
     * Update each element in scene graph or fetch for destruction
     */
    Vector<ptr<gfx::scene::SceneModel>> markedForDestruction {};
    graph.traversal(
        [scene = this, &markedForDestruction](auto* node_) {

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
        }
    );

    /**
     * Destroy every fetched marked node
     */
    for (auto&& marked : markedForDestruction) {

        [[maybe_unused]] const auto result { graph.pop(marked) };

        #ifdef _DEBUG
        assert(result);
        #endif

        delete marked;
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
            graph.push(static_cast<const ptr<gfx::scene::SceneModel>>(entry.second));
        }
    }
}

const hg::ptr<RenderSceneGraph> RevScene::renderGraph() noexcept {
    return &_renderGraph;
}

hg::nmpt<hg::engine::gfx::SceneResourcePool> RevScene::getSceneResourcePool() const noexcept {
    return _resourcePool.get();
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

    auto* gfxModel = static_cast<ptr<gfx::scene::SceneModel>>(model);
    _cached.emplace_back(gfx::GfxSceneTag {}, gfxModel);
    return true;
}

bool RevScene::removeNode(const ptr<SceneComponent> node_) {
    return false;
}

bool RevScene::removeNodeCached(const ptr<SceneComponent> node_) noexcept {
    return false;
}

hg::ptr<hg::IComponentRegisterContext> RevScene::registerContext() noexcept {
    return static_cast<hg::ptr<hg::IComponentRegisterContext>>(this);
}

void RevScene::add(const ptr<ActorComponent> component_) {
    // Warning: Unsafe
    addNodeCached(static_cast<const ptr<SceneComponent>>(component_));
}

void RevScene::add(cref<ComponentHierarchy> hierarchy_) {
    throw NotImplementedException {};
}
