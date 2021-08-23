#pragma once

#include <Engine.Common/Concurrent/SpinLock.hpp>
#include "CommandPool.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Texture/TextureBuffer.hpp"
#include "../Pipeline/RenderPass.hpp"
#include "../GraphicPipeline.hpp"

namespace ember::engine::gfx {

    class CommandBuffer {
    protected:
        friend ember::engine::gfx::CommandPool;

        /**
         * Constructor
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param  pool_ The pool.
         * @param  vkCmd_ The vk command.
         */
        CommandBuffer(ptr<CommandPool> pool_, const vk::CommandBuffer& vkCmd_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 22.11.2020
         */
        ~CommandBuffer() noexcept = default;

        /**
         * Begins recording
         *
         * @author Julius
         * @date 23.11.2020
         */
        void begin();

        /**
         * Begins render pass
         *
         * @author Julius
         * @date 12.12.2020
         *
         * @param  renderPass_ The render pass.
         * @param  framebuffer_ The framebuffer.
         * @param  inline_ (Optional) True to inline.
         */
        void beginRenderPass(const pipeline::RenderPass& renderPass_, const Framebuffer& framebuffer_,
            bool inline_ = true);

        void bindDescriptor(const vector<vk::DescriptorSet>& descriptors_);

        /**
         * Bind index buffer
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @param  buffer_ The buffer.
         * @param  offset_ The offset.
         */
        void bindIndexBuffer(const IndexBuffer& buffer_, u64 offset_);

    private:
        /**
         * Pipeline
         */
        vk::PipelineLayout _pipelineLayout;

    public:

        /**
         * Bind pipeline
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @param  pipeline_ The pipeline.
         */
        void bindPipeline(ptr<GraphicPipeline> pipeline_);

        /**
         * Bind vertex buffer
         *
         * @author Julius
         * @date 10.02.2021
         *
         * @param  binding_ The binding.
         * @param  buffer_ The buffer.
         * @param  offset_ The offset.
         */
        void bindVertexBuffer(const u32 binding_, cref<Buffer> buffer_, u64 offset_);

        /**
         * Bind vertex buffer
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @param  binding_ The binding.
         * @param  buffer_ The buffer.
         * @param  offset_ The offset.
         */
        void bindVertexBuffer(const u32 binding_, const VertexBuffer& buffer_, u64 offset_);

        /**
         * Blits
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param 		   blit_ The blit.
         * @param [in,out] src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         */
        void blit(vk::ImageBlit blit_, TextureBuffer& src_, TextureBuffer& dst_);

        /**
         * Copies the buffer
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         * @param 		   region_ The region.
         */
        void copyBuffer(const Buffer& src_, Buffer& dst_, const vk::BufferCopy& region_);

        /**
         * Copies the buffer to image
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         * @param 		   region_ The region.
         */
        void copyBufferToImage(const Buffer& src_, TextureBuffer& dst_, const vk::BufferImageCopy& region_);

        /**
         * Copies the image
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         * @param 		   region_ The copy.
         */
        void copyImage(const TextureBuffer& src_, TextureBuffer& dst_, const vk::ImageCopy& region_);

        /**
         * Copies the image to buffer
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param 		   src_ Source for the.
         * @param [in,out] dst_ Destination for the.
         * @param 		   region_ The region.
         */
        void copyImageToBuffer(const TextureBuffer& src_, Buffer& dst_, const vk::BufferImageCopy& region_);

        /**
         * Draws
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @param  instanceCount_ Number of instances.
         * @param  instanceOffset_ The instance offset.
         * @param  vertexCount_ Number of vertices.
         * @param  vertexOffset_ The vertex offset.
         */
        void draw(u32 instanceCount_, u32 instanceOffset_, u32 vertexCount_, u32 vertexOffset_);

        /**
         * Draw indexed
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @param  instanceCount_ Number of instances.
         * @param  instanceOffset_ The instance offset.
         * @param  indexCount_ Number of indexes.
         * @param  indexOffset_ The index offset.
         * @param  vertexOffset_ The vertex offset.
         */
        void drawIndexed(u32 instanceCount_, u32 instanceOffset_, u32 indexCount_, u32 indexOffset_, u32 vertexOffset_);

        /**
         * Draw indirect
         *
         * @author Julius
         * @date 12.12.2020
         *
         * @param  buffer_ The buffer.
         * @param  count_ Number of.
         * @param  offset_ (Optional) The offset.
         */
        void drawIndirect(const TypeBuffer<vk::DrawIndirectCommand>& buffer_, u32 count_, u32 offset_ = 0);

        /**
         * Draw indexed indirect
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @param  buffer_ The buffer.
         * @param  count_ Number of.
         * @param  offset_ (Optional) The offset.
         */
        void drawIndexedIndirect(const TypeBuffer<vk::DrawIndexedIndirectCommand>& buffer_, u32 count_,
            u32 offset_ = 0);

        /**
         * Ends recording
         *
         * @author Julius
         * @date 23.11.2020
         */
        void end();

        /**
         * Ends render pass
         *
         * @author Julius
         * @date 12.12.2020
         */
        void endRenderPass();

        /**
         * Resets this 
         *
         * @author Julius
         * @date 22.11.2020
         */
        void reset();

    private:
        /**
         * Command Pool
         */
        ptr<CommandPool> _pool;

    public:
        /**
         * Gets the CommandPool
         *
         * @author Julius
         * @date 09.12.2020
         *
         * @returns A ptr&lt;CommandPool&gt;
         */
        [[nodiscard]] ptr<CommandPool> pool() noexcept;

    public:
        /**
         * Gets a mutable reference to synchronization lock
         *
         * @author Julius
         * @date 25.11.2020
         */
        [[nodiscard]] ember::concurrent::UnfairSpinLock& lck() const;

        /**
         * Submits this to the CommandQueue and waits until it's finished
         *
         * @author Julius
         * @date 09.12.2020
         */
        void submitWait();

        /**
         * Releases this 
         *
         * @author Julius
         * @date 20.12.2020
         */
        void release();

    private:
        /**
         * Vulkan API
         */
        vk::CommandBuffer _vkCmd;

    public:
        /**
         * Vk command buffer
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @returns A reference to a const vk::CommandBuffer.
         */
        [[nodiscard]] const vk::CommandBuffer& vkCommandBuffer() const noexcept;
    };
}
