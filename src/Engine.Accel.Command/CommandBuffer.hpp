#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>

#include "CommandBufferStructs.hpp"

namespace hg::engine::accel {
    class ComputePass;
    class GraphicsPass;
    class ComputePipeline;
    class GraphicsPipeline;
}

namespace hg::engine::accel {
    class AccelCommandBuffer :
        public ::hg::engine::gfx::CommandBuffer {
    public:
        using this_type = AccelCommandBuffer;

    public:
        AccelCommandBuffer(mref<::hg::engine::gfx::CommandBuffer> commandBuffer_) noexcept;

        AccelCommandBuffer(cref<this_type>) = delete;

        AccelCommandBuffer(mref<this_type> other_) noexcept;

        ~AccelCommandBuffer() noexcept;

    public:
        [[nodiscard]] bool initialized() const noexcept {
            return _initialized;
        }

        [[nodiscard]] bool valid() const noexcept {
            return _valid;
        }

        [[nodiscard]] bool recording() const noexcept {
            return _recording;
        }

        [[nodiscard]] bool isFaf() const noexcept {
            return _faf;
        }

        [[nodiscard]] bool root() const noexcept {
            return _root;
        }

    public:
        /**
         * Used for internal submission to native queues
         *
         * @details Will release control of submission to external handled instance.
         *  Therefore this is primarily used from natives batches, as they are controlling
         *  the combined submission of multiple command buffers at once.
         */
        void commit();

    public:
        void bindComputePass(nmpt<const ComputePass> computePass_);

        void bindRenderPass(mref<BindRenderPassStruct> bindRenderPass_);

    public:
        void bindPipeline(nmpt<const ComputePipeline> computePipeline_);

        void bindPipeline(nmpt<const GraphicsPipeline> graphicsPipeline_);
    };
}
