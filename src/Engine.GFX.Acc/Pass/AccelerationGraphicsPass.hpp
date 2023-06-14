#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationGraphicsPass :
        public AccelerationPass {
    public:
        using this_type = AccelerationGraphicsPass;

    public:
        ~AccelerationGraphicsPass() override = default;
    };
}
