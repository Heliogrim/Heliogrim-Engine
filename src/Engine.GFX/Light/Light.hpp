#pragma once
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    struct ALIGNED(GlslLight, alignof(float)) {
        // color * intensity
        math::fvec4 luminance;
        math::fvec4 position;
        math::fvec4 direction;

        union {
            float limit;
            math::fvec4 dummy;
        };
    };

    //static_assert(sizeof(GlslLight) == sizeof(float) * 10);
    static_assert(sizeof(GlslLight) == sizeof(float) * 16);

    /**/

    struct ALIGNED(GlslSceneLightInfo, alignof(u32)) {
        u32 lightCount;
    };

    static_assert(sizeof(GlslSceneLightInfo) == sizeof(u32));
}
