#pragma once

#include "../RenderResource.hpp"

namespace hg::engine::accel {
    class ComputePipeline;
    class GraphicsPipeline;
}

namespace hg::engine::render::cmd {
    typedef RCmdRef<smr<const accel::ComputePipeline>> ComputePipelineRComRef;
    typedef RCmdRef<smr<const accel::GraphicsPipeline>> GraphicsPipelineRComRef;
}
