#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>

#include "../Command/CommandBatch.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class RenderPass;
    class RenderPassState;
    class RenderStage;
}

namespace ember::engine::gfx {

    enum class RenderPipelineValidationResult : u8 {
        eSuccess = 0x0,
        eFailedRequired,
        eFailedOrder
    };

    class RenderPipeline {
    public:
        using this_type = RenderPipeline;

    public:
        RenderPipeline();

        RenderPipeline(cref<this_type>) = delete;

        RenderPipeline(mref<this_type>) = delete;

        ~RenderPipeline();

    public:
        void destroy();

        void setup();

    public:
        void allocate(const ptr<const RenderPass> invocation_, _Inout_ const ptr<RenderPassState> state_);

        void free(const ptr<const RenderPass> invocation_, _Inout_ const ptr<RenderPassState> state_);

    public:
        void invoke(const ptr<const RenderPass> ctx_, ref<CommandBatch> batch_) const;

    public:
        void push(cref<sptr<RenderStage>> stage_);

        bool pop(cref<sptr<RenderStage>> stage_);

        [[nodiscard]] RenderPipelineValidationResult validate() const noexcept;
    };
}
