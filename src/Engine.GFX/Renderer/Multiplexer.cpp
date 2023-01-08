#include "Multiplexer.hpp"

#if _DEBUG
#include <cassert>
#endif

#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>

#include "RenderStageNode.hpp"
#include "Engine.GFX/Scene/SceneNodeModel.hpp"
#include "Engine.Reflect/EmberReflect.hpp"
#include "../Device/Device.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

Multiplexer::Multiplexer() = default;

Multiplexer::~Multiplexer() = default;

void Multiplexer::setup(cref<sptr<Device>> device_) {

    /**
     * Forwarding setup call to all registered nodes
     */
    for (auto& node : _nodes) {
        node->setup(device_);
    }
}

void Multiplexer::destroy() {

    /**
     * Forwarding destroy call to all registered nodes in reverse order
     */
    for (auto it { _nodes.rbegin() }; it != _nodes.rend(); ++it) {

        auto& node { *it };

        node->destroy();

        /**
         * Delete instance cause Multiplexer takes ownership of nodes while pushing
         */
        delete node;
        node = nullptr;
    }
}

bool Multiplexer::allocate(const ptr<HORenderPass> renderPass_) {

    bool succeeded = true;

    /**
     * Iterate over all RenderStageNodes to allocate HORenderPass related resources
     */
    for (auto& node : _nodes) {
        #if _DEBUG
        const auto success { node->allocate(renderPass_) };
        assert(success);
        #else
        succeeded = node->allocate(renderPass_) && succeeded;
        #endif
    }

    return succeeded;
}

void Multiplexer::free(const ptr<HORenderPass> renderPass_) {

    /**
     * Iterate over all RenderStageNodes in reverse order to free HORenderPass related resources
     */
    for (auto it { _nodes.rbegin() }; it != _nodes.rend(); ++it) {
        #if _DEBUG
        assert((*it)->free(renderPass_));
        #else
        (*it)->free(renderPass_);
        #endif
    }
}

void Multiplexer::dispatchNull(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {

    const auto it { _mesh.mapping.find(null_type_id) };

    /**
     * Early exit, cause there is no required invocation
     */
    if (it == _mesh.mapping.end()) {
        return;
    }

    auto& mapped { it->second };
    for (const auto& node : mapped) {
        node->invoke(renderPass_, stagePass_, nullptr);
    }
}

void Multiplexer::dispatchAny(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_, const non_owning_rptr<SceneNodeModel> model_) const {

    // TODO: Get model type / for now just anything but null_type_id
    const type_id modelType { model_->getClass()->typeId() };
    const _STD array<type_id, 1> types { modelType };

    // Warning: This is just a dirty workaround saving us 500FPS when just iterating 1024 elements without any action\
    //  CompactSet seams to be very expensive to create, so we should replace it with a better solution
    thread_local static CompactSet<ptr<RenderStageNode>> effected {};
    effected.clear();

    // TODO: Resolve class/type hierarchy to fetch every type id involved

    for (const auto& type : types) {
        auto it { _mesh.mapping.find(type) };

        /**
         * Ignore if type not found
         */
        if (it == _mesh.mapping.end()) {
            return;
        }

        /**
         * Attention: Be aware that this might change invocation order of nodes based cause by the class hierarchy
         */
        const auto& mapped { it->second };
        for (const auto& node : mapped) {

            if (effected.contains(node)) {
                continue;
            }

            // TODO: Before invoking the render nodes, collect all effected nodes to optimize cache usage with small loops
            node->invoke(renderPass_, stagePass_, model_);
            effected.insert(node);
        }
    }
}

void Multiplexer::attach(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {
    /**
     * Iterate forward every node with enforced order
     */
    for (const auto* node : _nodes) {
        node->before(renderPass_, stagePass_);
    }
}

void Multiplexer::dispatch(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {

    /**
     * Check for root invocation where model_ is not provided (nullptr)
     */
    if (model_ == nullptr) {
        dispatchNull(renderPass_, stagePass_);

    } else {
        dispatchAny(renderPass_, stagePass_, model_);
    }

}

void Multiplexer::detach(const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_) const {
    /**
     * Iterate reverse every node with enforced order
     */
    for (auto it { _nodes.rbegin() }; it != _nodes.rend(); ++it) {
        (*it)->after(renderPass_, stagePass_);
    }
}

void Multiplexer::push(mref<ptr<RenderStageNode>> node_) {

    /**
     * Query types to register to
     */
    const auto node { _STD move(node_) };
    const auto types { node_->modelTypes() };

    /**
     * Store into node container
     */
    _nodes.push_back(node);

    /**
     * Check for nullptr => root invocation / once a frame
     */
    if (types == nullptr) {
        _mesh.mapping[null_type_id].push_back(node);
        return;
    }

    /**
     * Otherwise we need to mesh all supported types
     */
    for (const auto& entry : *types) {
        _mesh.mapping[entry].push_back(node);
    }
}

bool Multiplexer::pop(cref<non_owning_rptr<RenderStageNode>> node_) {

    /**
     * Remove node from container
     */
    const auto remove { _STD remove(_nodes.begin(), _nodes.end(), node_) };
    if (remove == _nodes.end()) {
        return false;
    }

    /**
     * Use same type query as push -> faster remove instead of iterating over all entries
     *  // TODO: Check whether we should tidy empty mesh mappings for performance reason
     */
    const auto types { node_->modelTypes() };

    if (types == nullptr) {
        auto& container { _mesh.mapping.at(null_type_id) };
        const auto mrem { _STD remove(container.begin(), container.end(), node_) };
        container.erase(mrem, container.end());

    } else {

        for (const auto& type : *types) {
            auto& container { _mesh.mapping.at(type) };
            const auto mrem { _STD remove(container.begin(), container.end(), node_) };
            container.erase(mrem, container.end());
        }
    }

    /**
     * Erase after mesh got updated
     */
    _nodes.erase(remove, _nodes.end());
    return true;
}

void Multiplexer::insert(const node_container_type::iterator where_, cref<sptr<RenderStageNode>> node_) {
    throw NotImplementedException();
}
