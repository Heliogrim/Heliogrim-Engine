#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::reflow {
    struct ReflowPassState {
        /* Bottom-Up pass requested size without relative resolving */
        math::vec2 prefetchedSize;

        /* Forwarded Top-Down pass with relative resolving and compound arrangement*/
        math::vec2 referenceSize;

        /* Computational size request */
        math::vec2 cachedPreservedSize;

        /* Assigned layout parameters */
        math::vec2 layoutOffset;
        math::vec2 layoutSize;
    };
}
