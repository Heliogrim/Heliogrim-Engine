#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "../Command/CommandBatch.hpp"

#include "__fwd.hpp"

namespace hg::engine::gfx::render {
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
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @param stage_ The current RenderStage instance.
         */
        RenderStagePass(const non_owning_rptr<RenderStage> stage_);

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

    private:
        CommandBatch _batch;

    public:
        [[nodiscard]] cref<CommandBatch> batch() const noexcept;

        [[nodiscard]] ref<CommandBatch> batch() noexcept;
    };
}
