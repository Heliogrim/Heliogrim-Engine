#pragma once

#include "AccelerationPipeline.hpp"

namespace hg::engine::accel {
    class GraphicsPipeline :
        public InheritMeta<GraphicsPipeline, AccelerationPipeline> {
    public:
        using this_type = GraphicsPipeline;

    protected:
        GraphicsPipeline(mref<smr<const AccelerationEffect>> effect_);

    public:
        ~GraphicsPipeline() override = default;
    };
}
