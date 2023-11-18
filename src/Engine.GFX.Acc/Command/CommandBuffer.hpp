#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>

#include "CommandBufferStructs.hpp"

namespace hg::engine::gfx::acc {
    class ComputePass;
    class GraphicsPass;
    class ComputePipeline;
    class GraphicsPipeline;
}

namespace hg::engine::gfx {
    class AccelCommandBuffer :
        public ::hg::engine::gfx::CommandBuffer {
    public:
        using this_type = AccelCommandBuffer;
        using ::hg::engine::gfx::CommandBuffer::CommandBuffer;

    public:
        AccelCommandBuffer(mref<::hg::engine::gfx::CommandBuffer> commandBuffer_) noexcept;

        constexpr ~AccelCommandBuffer() noexcept = default;

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
        void bindComputePass(const nmpt<const acc::ComputePass> computePass_);

        void bindRenderPass(mref<acc::BindRenderPassStruct> bindRenderPass_);

    public:
        void bindPipeline(const nmpt<const acc::ComputePipeline> computePipeline_);

        void bindPipeline(const nmpt<const acc::GraphicsPipeline> graphicsPipeline_);
    };
}
