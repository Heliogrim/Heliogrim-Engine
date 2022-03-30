#pragma once
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderStage;
}

namespace ember::engine::gfx {

    class RenderStagePass {
    public:
        using this_type = RenderStagePass;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        RenderStagePass() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        ~RenderStagePass() = default;

    private:
        ptr<RenderStage> _stage;

    public:
        [[nodiscard]] const ptr<const RenderStage> stage() const noexcept;
    };
}
