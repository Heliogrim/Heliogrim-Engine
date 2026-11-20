#pragma once
#include "AccelerationPass.hpp"

#include <Engine.Common/__macro.hpp>

namespace hg::engine::accel {
    class macro_novtable GraphicsPass :
        public InheritMeta<GraphicsPass, AccelerationPass> {
    public:
        using this_type = GraphicsPass;
    };
}
