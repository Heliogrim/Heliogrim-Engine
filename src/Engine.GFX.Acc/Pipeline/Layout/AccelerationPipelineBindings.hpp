#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "AccelerationBindingLayout.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationPipelineBindings {
        using this_type = AccelerationPipelineBindings;

    public:
        Vector<AccelerationBindingLayout> layouts;
    };
}
