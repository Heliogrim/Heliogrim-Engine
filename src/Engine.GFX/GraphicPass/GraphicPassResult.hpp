#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "../Command/CommandBuffer.hpp"

namespace hg::engine::gfx {
    struct GraphicPassResult {
        /**
         *
         */
        Vector<CommandBuffer> recordedBuffer;
    };
}
