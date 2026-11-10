#pragma once

#include "__fwd.hpp"

namespace hg::engine::gfx::loader {
    enum class TextureStreamOp : bool {
        eLoad = true,
        eUnload = false
    };

    struct TextureStreamOptions {
        TextureStreamOp op;
        /**/
        u32 layer;
        u32 mip;
        math::uivec3 offset;
        math::uivec3 extent;
    };
}
