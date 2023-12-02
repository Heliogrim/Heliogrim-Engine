#pragma once
#include "AccelerationPass.hpp"

namespace hg::engine::accel {
    class __declspec(novtable) GraphicsPass :
        public InheritMeta<GraphicsPass, AccelerationPass> {
    public:
        using this_type = GraphicsPass;
    };
}
