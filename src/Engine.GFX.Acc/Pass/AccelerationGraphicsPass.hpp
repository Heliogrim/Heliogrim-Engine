#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationGraphicsPass :
        public InheritMeta<AccelerationGraphicsPass, AccelerationPass> {
    public:
        using this_type = AccelerationGraphicsPass;

    protected:
        AccelerationGraphicsPass(mref<smr<AccelerationEffect>> effect_);

    public:
        ~AccelerationGraphicsPass() override = default;
    };
}
