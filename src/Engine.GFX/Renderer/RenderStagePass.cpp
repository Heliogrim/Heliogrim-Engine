#include "RenderStagePass.hpp"

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

RenderStagePass::RenderStagePass(const non_owning_rptr<RenderStage> stage_) :
    _stage(stage_),
    _batch() {}

const ptr<const RenderStage> RenderStagePass::stage() const noexcept {
    return _stage;
}

cref<CommandBatch> RenderStagePass::batch() const noexcept {
    return _batch;
}

ref<CommandBatch> RenderStagePass::batch() noexcept {
    return _batch;
}
