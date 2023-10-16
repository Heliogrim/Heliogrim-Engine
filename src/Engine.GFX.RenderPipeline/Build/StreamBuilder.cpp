#include "StreamBuilder.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

uptr<RenderPipeline> StreamBuilder::operator()(
    mref<uptr<RenderPipeline>> iteratee_,
    mref<uptr<graph::RuntimeGraph>> runtimeGraph_
) const {
    return iteratee_;
}
