#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Command/CommandBatch.hpp"
#include "../Renderer/RenderPassState.hpp"

namespace ember::engine::gfx {
    struct GraphicPassStageContext {
        ref<CommandBatch> batch;
    };
}
