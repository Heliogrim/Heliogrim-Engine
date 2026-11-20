#pragma once
#include <Engine.Common/__macro.hpp>

#include "AccelerationPass.hpp"

namespace hg::engine::accel {
    class macro_novtable MeshPass :
        public InheritMeta<MeshPass, AccelerationPass> {
    public:
        using this_type = MeshPass;
    };
}
