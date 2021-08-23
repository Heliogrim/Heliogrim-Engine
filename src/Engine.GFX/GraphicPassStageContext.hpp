#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Command/CommandBatch.hpp"

namespace ember::engine::gfx {

    struct GraphicPassStageContext {
        ref<CommandBatch> batch;
    };
}
