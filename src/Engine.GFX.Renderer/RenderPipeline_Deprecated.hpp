#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration
     */
    class Device;
}

namespace hg::engine::gfx::render {
    enum class RenderPipelineValidationResult : u8 {
        eSuccess = 0x0,
        eFailedRequired,
        eFailedOrder,
        eFailedUnique
    };

    class RenderPipeline_Deprecated {
    public:
        using this_type = RenderPipeline_Deprecated;

    public:
        RenderPipeline_Deprecated();

        RenderPipeline_Deprecated(cref<this_type>) = delete;

        RenderPipeline_Deprecated(mref<this_type>) = delete;

        ~RenderPipeline_Deprecated();

    public:
        void destroy();

        void setup(cref<sptr<Device>> device_);

    public:
        void allocate(_Inout_ const ptr<HORenderPass> renderPass_);

        void free(_Inout_ const ptr<HORenderPass> renderPass_);

    public:
        void invoke(const non_owning_rptr<HORenderPass> renderPass_) const;

    private:
        /**
         * The sorted collection of defined render stages
         */
        Vector<sptr<RenderStage>> _stages;

    public:
        // Warning: We should not access stages
        [[nodiscard]] cref<Vector<sptr<RenderStage>>> stages() const noexcept;

        RenderPipelineValidationResult push(cref<sptr<RenderStage>> stage_);

        bool pop(cref<sptr<RenderStage>> stage_);

        [[nodiscard]] RenderPipelineValidationResult validate() const noexcept;

    private:
        /**
         * The api/graphic device this render pipeline is associated with
         */
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;
    };
}
