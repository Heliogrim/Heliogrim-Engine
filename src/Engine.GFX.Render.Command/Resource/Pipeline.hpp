#pragma once

#include "../RenderResource.hpp"

namespace hg::engine::gfx::acc {
    class ComputePipeline;
    class GraphicsPipeline;
}

namespace hg::engine::gfx::render::cmd {
    typedef RCmdRef<smr<const acc::ComputePipeline>> ComputePipelineRComRef;
    typedef RCmdRef<smr<const acc::GraphicsPipeline>> GraphicsPipelineRComRef;
}
