#pragma once

#include "GraphicsPipeline.hpp"

namespace hg::engine::accel {
    class DxGraphicsPipeline :
        public GraphicsPipeline {
    public:
        using this_type = DxGraphicsPipeline;

    public:
        ~DxGraphicsPipeline() override = default;
    };
}
