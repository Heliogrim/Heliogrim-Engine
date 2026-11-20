#pragma once
#include <Engine.Common/__macro.hpp>

#include "AccelerationPass.hpp"

namespace hg::engine::accel {
    class macro_novtable RaytracingPass :
        public InheritMeta<RaytracingPass, AccelerationPass> {
    public:
        using this_type = RaytracingPass;
    };
}
