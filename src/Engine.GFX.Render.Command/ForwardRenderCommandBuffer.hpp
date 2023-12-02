#pragma once

#include <Engine.Accel.Effect/__fwd.hpp>

#include "RenderCommandBuffer.hpp"
#include "Resource/Pipeline.hpp"

namespace hg::engine::gfx::render::cmd {
    class ForwardRenderCommandBuffer final :
        public RenderCommandBuffer {
    public:
        using this_type = ForwardRenderCommandBuffer;

    public:
        ForwardRenderCommandBuffer() noexcept;

        ForwardRenderCommandBuffer(mref<this_type> other_) noexcept;

        ForwardRenderCommandBuffer(cref<this_type>) = delete;

        ~ForwardRenderCommandBuffer() noexcept override = default;

    public:
        /* Forward Render Commands */
        [[nodiscard]] ComputePipelineRComRef createComputePipeline(
            mref<smr<void>> effect_
        ) = delete;

        [[nodiscard]] GraphicsPipelineRComRef createGraphicsPipeline(
            mref<smr<void>> effect_,
            nmpt<void> specification_,
            nmpt<void> profile_
        ) = delete;

    public:
        /* Immediate Commands */

        [[nodiscard]] ComputePipelineRComRef createComputePipelineImmediately(
            mref<smr<void>> effect_
        ) = delete;

        [[nodiscard]] GraphicsPipelineRComRef createGraphicsPipelineImmediately(
            mref<smr<const accel::AccelerationEffect>> effect_,
            nmpt<void> specification_,
            nmpt<void> profile_
        );
    };
}
