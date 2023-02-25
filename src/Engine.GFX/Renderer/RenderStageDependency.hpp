#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace ember::engine::gfx::render {
    enum class RenderStageOrder : u8 {
        eIgnore,
        ePredecessor,
        eSuccessor
    };

    struct RenderStageDependency {
        ptr<RenderStage> stage;
        RenderStageOrder order;
        bool required;
    };
}
