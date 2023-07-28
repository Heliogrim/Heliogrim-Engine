#include "RenderPass.hpp"

#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

RenderPass::RenderPass(
    u32 runtimeVersion_,
    mref<uptr<RenderPipeline>> pipeline_,
    mref<uptr<pipeline::State>> state_
) noexcept :
    _rtVer(runtimeVersion_),
    _pipeline(_STD move(pipeline_)),
    _state(_STD move(state_)) {}

RenderPass::~RenderPass() noexcept = default;

bool RenderPass::alloc() {
    return _pipeline->alloc(_state.get());
}

bool RenderPass::realloc() {
    if (_pipeline->isReallocSupported()) {
        return _pipeline->realloc(_state.get());
    }

    const auto success = free();
    return success ? alloc() : false;
}

bool RenderPass::free() {
    return _pipeline->free(_state.get());
}

RenderPassResult RenderPass::operator()() {
    // TODO:
    return RenderPassResult::eFailed;
}
