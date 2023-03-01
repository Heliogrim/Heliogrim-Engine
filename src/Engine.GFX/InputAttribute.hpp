#pragma once
#include <Engine.Common/Types.hpp>
#include "TextureFormat.hpp"

namespace hg::engine::gfx {
    struct InputAttribute {
        /**
         * Shader Location Binding
         */
        u32 location;

        /**
         * Data Format
         */
        TextureFormat format;

        /**
         * Data Inner Offset
         */
        u32 dataOffset;
    };
}
