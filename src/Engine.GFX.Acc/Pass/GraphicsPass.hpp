#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class GraphicsPass :
        public InheritMeta<GraphicsPass, AccelerationPass> {
    public:
        using this_type = GraphicsPass;

    protected:
        GraphicsPass(mref<smr<AccelerationEffect>> effect_);

    public:
        ~GraphicsPass() override = default;
    };
}
