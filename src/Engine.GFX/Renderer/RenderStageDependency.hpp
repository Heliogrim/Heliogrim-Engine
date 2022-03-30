#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderStage;
}

namespace ember::engine::gfx {

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
