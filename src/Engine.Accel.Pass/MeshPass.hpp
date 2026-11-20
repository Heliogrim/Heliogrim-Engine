#pragma once
#include "AccelerationPass.hpp"

#include <Engine.Common/__macro.hpp>

namespace hg::engine::accel {
    class macro_novtable MeshPass :
        public InheritMeta<MeshPass, AccelerationPass> {
    public:
        using this_type = MeshPass;
    };
}
