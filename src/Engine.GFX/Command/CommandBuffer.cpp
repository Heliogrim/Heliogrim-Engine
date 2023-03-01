#include "CommandBuffer.hpp"

#include "CommandPool.hpp"
#include "CommandQueue.hpp"
#include "../Framebuffer/Framebuffer.hpp"
#include "../VkComputePipeline.hpp"
#include "../VkFixedPipeline.hpp"

using namespace hg::engine::gfx;
using namespace hg;

CommandBuffer::CommandBuffer(ptr<CommandPool> pool_, const vk::CommandBuffer& vkCmd_) noexcept :
    _pool(pool_),
    _vkCmd(vkCmd_) {}

void CommandBuffer::begin() {
    const vk::CommandBufferBeginInfo info {
        vk::CommandBufferUsageFlagBits::eSimultaneousUse
    };
    _vkCmd.begin(info);
}

void CommandBuffer::beginRenderPass(
    const pipeline::LORenderPass& renderPass_,
    const Framebuffer& framebuffer_,
    bool inline_
) {

    vk::RenderPassBeginInfo info = renderPass_.vkBeginInfo();
    info.framebuffer = framebuffer_.vkFramebuffer();
    info.renderArea = vk::Rect2D { { 0, 0 }, { framebuffer_.width(), framebuffer_.height() } };

    _vkCmd.beginRenderPass(
        info,
        inline_ ? vk::SubpassContents::eInline : vk::SubpassContents::eSecondaryCommandBuffers
    );
}

void CommandBuffer::bindDescriptor(const Vector<vk::DescriptorSet>& descriptors_) {
    _vkCmd.bindDescriptorSets(
        _pipelineBindPoint,
        _pipelineLayout,
        0,
        static_cast<u32>(descriptors_.size()),
        descriptors_.data(),
        0,
        nullptr
    );
}

void CommandBuffer::bindDescriptor(const u32 id_, cref<vk::DescriptorSet> descriptor_) {

    _vkCmd.bindDescriptorSets(
        _pipelineBindPoint,
        _pipelineLayout,
        id_,
        1,
        &descriptor_,
        0,
        nullptr
    );
}

void CommandBuffer::bindIndexBuffer(const IndexBuffer& buffer_, u64 offset_) {
    _vkCmd.bindIndexBuffer(
        buffer_.buffer,
        offset_,
        vk::IndexType::eUint32
    );
}

void CommandBuffer::bindIndexBuffer(const ptr<const VirtualBuffer> buffer_, u64 offset_) {
    _vkCmd.bindIndexBuffer(
        buffer_->vkBuffer(),
        offset_,
        vk::IndexType::eUint32
    );
}

void CommandBuffer::bindPipeline(ptr<ComputePipeline> pipeline_) {

    const auto vkp = static_cast<ptr<VkComputePipeline>>(pipeline_);

    _vkCmd.bindPipeline(vk::PipelineBindPoint::eCompute, vkp->vkPipeline());

    _pipelineLayout = vkp->vkLayout();
    _pipelineBindPoint = vk::PipelineBindPoint::eCompute;
}

void CommandBuffer::bindPipeline(ptr<GraphicPipeline> pipeline_, cref<Viewport> viewport_) {
    //const auto& viewport = pipeline_->viewport();

    vk::Viewport vkViewport {
        static_cast<float>(viewport_.offsetX()),
        static_cast<float>(viewport_.offsetY()),
        static_cast<float>(viewport_.width()),
        static_cast<float>(viewport_.height()),
        viewport_.minDepth(),
        viewport_.maxDepth()
    };

    vk::Rect2D vkScissor {
        { 0, 0 },
        { viewport_.width(), viewport_.height() }
    };

    const auto vkp = static_cast<ptr<VkFixedPipeline>>(pipeline_);

    _vkCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, vkp->vkPipeline());
    _vkCmd.setScissor(0, 1, &vkScissor);
    _vkCmd.setViewport(0, 1, &vkViewport);

    _pipelineLayout = vkp->vkLayout();
    _pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, cref<Buffer> buffer_, u64 offset_) {
    _vkCmd.bindVertexBuffers(
        binding_,
        1,
        &buffer_.buffer,
        &offset_
    );
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, const VertexBuffer& buffer_, u64 offset_) {
    _vkCmd.bindVertexBuffers(
        binding_,
        1,
        &buffer_.buffer,
        &offset_
    );
}

void CommandBuffer::bindVertexBuffer(const u32 binding_, const ptr<const VirtualBuffer> buffer_, u64 offset_) {
    _vkCmd.bindVertexBuffers(binding_, 1ui32, &buffer_->vkBuffer(), &offset_);
}

void CommandBuffer::copyBuffer(const Buffer& src_, Buffer& dst_, const vk::BufferCopy& region_) {
    _vkCmd.copyBuffer(
        src_.buffer,
        dst_.buffer,
        1,
        &region_
    );
}

void CommandBuffer::copyBufferToImage(
    const Buffer& src_,
    TextureBuffer& dst_,
    const vk::BufferImageCopy& region_
) {
    _vkCmd.copyBufferToImage(
        src_.buffer,
        dst_.image(),
        dst_._vkLayout,
        1,
        &region_
    );
}

void CommandBuffer::copyImage(
    const TextureBuffer& src_,
    TextureBuffer& dst_,
    const vk::ImageCopy& region_
) {
    _vkCmd.copyImage(
        src_.image(),
        src_._vkLayout,
        dst_.image(),
        dst_._vkLayout,
        1,
        &region_
    );
}

void CommandBuffer::copyImageToBuffer(
    const TextureBuffer& src_,
    Buffer& dst_,
    const vk::BufferImageCopy& region_
) {
    _vkCmd.copyImageToBuffer(
        src_.image(),
        src_._vkLayout,
        dst_.buffer,
        1,
        &region_
    );
}

void CommandBuffer::draw(u32 instanceCount_, u32 instanceOffset_, u32 vertexCount_, u32 vertexOffset_) {
    _vkCmd.draw(vertexCount_, instanceCount_, vertexOffset_, instanceOffset_);
}

void CommandBuffer::drawIndexed(
    u32 instanceCount_,
    u32 instanceOffset_,
    u32 indexCount_,
    u32 indexOffset_,
    u32 vertexOffset_
) {
    _vkCmd.drawIndexed(indexCount_, instanceCount_, indexOffset_, vertexOffset_, instanceOffset_);
}

void CommandBuffer::drawIndirect(const TypeBuffer<vk::DrawIndirectCommand>& buffer_, u32 count_, u32 offset_) {
    _vkCmd.drawIndirect(buffer_.buffer, offset_, count_, sizeof(vk::DrawIndirectCommand));
}

void CommandBuffer::drawIndexedIndirect(
    const TypeBuffer<vk::DrawIndexedIndirectCommand>& buffer_,
    u32 count_,
    u32 offset_
) {
    _vkCmd.drawIndexedIndirect(buffer_.buffer, offset_, count_, sizeof(vk::DrawIndexedIndirectCommand));
}

void CommandBuffer::end() {
    _pipelineLayout = nullptr;
    _vkCmd.end();
}

void CommandBuffer::endRenderPass() {
    return _vkCmd.endRenderPass();
}

void CommandBuffer::reset() {
    _vkCmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

ptr<CommandPool> CommandBuffer::pool() noexcept {
    return _pool;
}

hg::concurrent::UnfairSpinLock& CommandBuffer::lck() const {
    return _pool->lck();
}

void CommandBuffer::submitWait() {
    _pool->queue()->submitWait(*this);
}

void CommandBuffer::release() {
    _pool->release(*this);
    _vkCmd = nullptr;
}

const vk::CommandBuffer& CommandBuffer::vkCommandBuffer() const noexcept {
    return _vkCmd;
}
