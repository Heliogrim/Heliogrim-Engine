#pragma once

#include "__fwd.hpp"

namespace ember::engine::gfx::loader {
    struct TextureStreamOptions {
        u32 layer;
        u32 mip;
        math::ivec3 offset;
        math::uivec3 extent;
    };
}
