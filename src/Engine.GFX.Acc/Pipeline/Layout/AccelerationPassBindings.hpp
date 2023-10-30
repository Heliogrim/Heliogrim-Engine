#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "AccelerationBindingLayout.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationPassBindings {
        using this_type = AccelerationPassBindings;

    public:
        Vector<AccelerationBindingLayout> layouts;
    };
}
