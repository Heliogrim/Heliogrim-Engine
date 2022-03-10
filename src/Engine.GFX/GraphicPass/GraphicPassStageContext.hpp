#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "../Command/CommandBatch.hpp"
#include "../Renderer/RenderInvocationState.hpp"

namespace ember::engine::gfx {

    struct GraphicPassStageContext {
        ref<CommandBatch> batch;

        cref<RenderInvocationState> state;
    };
}
