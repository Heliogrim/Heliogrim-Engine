#pragma once

#include <Engine.Common/Collection/List.hpp>

#include "Command/CommandBuffer.hpp"

namespace ember::engine::gfx {

    struct GraphicPassResult {
        /**
         *
         */
        vector<CommandBuffer> recordedBuffer;
    };
}
