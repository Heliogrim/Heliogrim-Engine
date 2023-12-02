#pragma once
#include "AccelerationPass.hpp"

namespace hg::engine::accel {
    class __declspec(novtable) MeshPass :
        public InheritMeta<MeshPass, AccelerationPass> {
    public:
        using this_type = MeshPass;
    };
}
