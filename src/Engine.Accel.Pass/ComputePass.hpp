#pragma once
#include "AccelerationPass.hpp"

namespace hg::engine::accel {
    class __declspec(novtable) ComputePass :
        public InheritMeta<ComputePass, AccelerationPass> {
    public:
        using this_type = ComputePass;
    };
}
