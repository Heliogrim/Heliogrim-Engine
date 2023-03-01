#include "RenderStageProcessor.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "Multiplexer.hpp"
#include "HORenderPass.hpp"
#include "RenderStagePass.hpp"
#include "../Scene/SceneNodeModel.hpp"
#include <Engine.Scene/IRenderScene.hpp>

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

RenderStageProcessor::RenderStageProcessor(const non_owning_rptr<const Multiplexer> multiplexer_) :
    _renderPass(nullptr),
    _stagePass(nullptr),
    _multiplexer(multiplexer_),
    _signal() {}

RenderStageProcessor::dispatch_result_type RenderStageProcessor::dispatch() const {

    /**
     * Clear Signal
     */
    _signal.clear(_STD memory_order::release);

    // TODO: Schedule asynchronous workload
    {
        /**
         * Get current render graph
         */
        auto graph { _renderPass->scene()->renderGraph() };

        /**
         * Pulse before invocation
         */
        _multiplexer->attach(_renderPass, _stagePass);

        /**
         * Preserve Null-Invocation
         */
        _multiplexer->dispatch(_renderPass, _stagePass, nullptr);

        /**
         * Traverse
         */
        graph->traversalBatched(
            0,
            [this](auto batch_, auto value_) -> bool {
                return (*this)(value_);
            }
        );

        /**
         * Pulse after invocation
         */
        _multiplexer->detach(_renderPass, _stagePass);
    }

    // Raise signal when finished
    _signal.test_and_set(_STD memory_order::release);

    /**
     * Build dispatch result
     */
    return &_signal;
}

bool RenderStageProcessor::operator()(const ptr<::hg::engine::scene::RenderGraph::node_type> node_) const {

    SCOPED_STOPWATCH

    /**
     * Checkout elements
     */
    auto** elements { node_->elements() };
    const auto elementCount { node_->exclusiveSize() };

    for (u64 i = 0; i < elementCount; ++i) {

        // TODO: Rework / Rewrite
        if (!(*elements)->markedAsDeleted()) [[likely]]
        {
            _multiplexer->dispatch(_renderPass, _stagePass, *elements);
        }

        ++elements;
    }

    return node_->inclusiveSize() && !node_->isLeaf();
}

void RenderStageProcessor::use(const non_owning_rptr<HORenderPass> renderPass_) noexcept {
    _renderPass = renderPass_;
}

void RenderStageProcessor::use(const non_owning_rptr<RenderStagePass> renderStagePass_) noexcept {
    _stagePass = renderStagePass_;
}

const ptr<const Multiplexer> RenderStageProcessor::multiplexer() const noexcept {
    return _multiplexer;
}
