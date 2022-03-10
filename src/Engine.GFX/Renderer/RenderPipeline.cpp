#include "RenderPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RenderPipeline::RenderPipeline() = default;

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::destroy() {
    /**
     *
     */
    for (auto it { _passes.rbegin() }; it != _passes.rend(); ++it) {
        if ((*it) != nullptr) {
            (*it)->destroy();
        }
    }
}

void RenderPipeline::setup() {
    /**
     *
     */
    for (auto* entry : _passes) {
        if (entry != nullptr) {
            entry->setup();
        }
    }
}

bool RenderPipeline::defineGraphicPass(const u8 graphicPassIndex_, const ptr<GraphicPass> graphicPass_) {

    if (_passes.size() < static_cast<size_t>(graphicPassIndex_) + 1) {
        _passes.reserve(static_cast<size_t>(graphicPassIndex_) + 1);

        for (auto i = _passes.size(); i <= graphicPassIndex_; ++i) {
            _passes.push_back(nullptr);
        }
    }

    _passes[graphicPassIndex_] = graphicPass_;
    return true;
}

void RenderPipeline::process(const ptr<RenderInvocation> invocation_, ref<CommandBatch> batch_) {
    /**
     *
     */
    for (u32 gpi { 0ui32 }; gpi < _passes.size(); ++gpi) {
        if (_passes[gpi] != nullptr) {
            _passes[gpi]->process(
                invocation_->scene()->renderGraph(),
                *invocation_->state(),
                batch_
            );
        }
    }
}

void RenderPipeline::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {
    /**
     *
     */
    for (const auto& entry : _passes) {
        if (entry != nullptr) {
            entry->allocateWith(invocation_, state_);
        }
    }
}

void RenderPipeline::freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) {
    /**
     *
     */
    for (auto it { _passes.rbegin() }; it != _passes.rend(); ++it) {
        if ((*it) != nullptr) {
            (*it)->freeWith(invocation_, state_);
        }
    }
}
