#pragma once
#include "AccelerationPass.hpp"

#include <Engine.Common/__macro.hpp>

namespace hg::engine::accel {
    class macro_novtable ComputePass :
        public InheritMeta<ComputePass, AccelerationPass> {
    public:
        using this_type = ComputePass;
    };
}
