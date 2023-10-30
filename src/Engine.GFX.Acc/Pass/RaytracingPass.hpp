#pragma once
#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) RaytracingPass :
        public InheritMeta<RaytracingPass, AccelerationPass> {
    public:
        using this_type = RaytracingPass;
    };
}
