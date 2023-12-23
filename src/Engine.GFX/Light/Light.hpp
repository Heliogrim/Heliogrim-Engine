#pragma once
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    struct ALIGNED(GlslLight, alignof(float)) {
        // color * intensity
        math::fvec3 luminance;
        math::fvec3 position;
        math::fvec3 direction;
        float limit;
    };

    static_assert(sizeof(GlslLight) == sizeof(float) * 10);
}
