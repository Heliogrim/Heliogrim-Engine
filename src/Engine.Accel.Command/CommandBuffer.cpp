#include "CommandBuffer.hpp"

#include <Engine.Accel.Pass/VkGraphicsPass.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>

using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

AccelCommandBuffer::AccelCommandBuffer(mref<CommandBuffer> commandBuffer_) noexcept :
    CommandBuffer(_STD move(commandBuffer_)) {}

void AccelCommandBuffer::commit() {
    assert(_valid);
    _valid = not _faf;
}

void AccelCommandBuffer::bindComputePass(const nmpt<const ComputePass> computePass_) {
    assert(false);
}

void AccelCommandBuffer::bindRenderPass(mref<BindRenderPassStruct> bindRenderPass_) {

    const auto& graphicsPass = static_cast<cref<VkGraphicsPass>>(bindRenderPass_.graphicsPass);

    Vector<vk::ClearValue> clearValues {};
    clearValues.reserve(graphicsPass._references.size());

    for (const auto& reference : graphicsPass._references) {
        if (
            reference.layout == vk::ImageLayout::eDepthAttachmentOptimal ||
            reference.layout == vk::ImageLayout::eDepthStencilAttachmentOptimal
        ) {
            clearValues.push_back(vk::ClearDepthStencilValue { 1.F, 0uL });
        } else {
            clearValues.push_back(vk::ClearColorValue { 0.F, 0.F, 0.F, 1.F });
        }
    }

    auto renderArea = vk::Rect2D {
        vk::Offset2D {}, vk::Extent2D { bindRenderPass_.framebuffer.width(), bindRenderPass_.framebuffer.height() }
    };
    vk::RenderPassBeginInfo rpbi {
        reinterpret_cast<VkRenderPass>(graphicsPass._vkGraphicsPass),
        bindRenderPass_.framebuffer.vkFramebuffer(),
        renderArea,
        static_cast<u32>(clearValues.size()),
        clearValues.data()
    };

    _vkCmd.beginRenderPass(rpbi, vk::SubpassContents::eInline);

    /* Warning: Temporary */
    _vkCmd.setScissor(0uL, 1uL, &renderArea /* Warning: alias scissor by default ~ will break ui renderer */);
    ::vk::Viewport viewport {
        0.F, 0.F, static_cast<float>(rpbi.renderArea.extent.width), static_cast<float>(rpbi.renderArea.extent.height),
        0.F, 1.F
    };
    _vkCmd.setViewport(0uL, 1uL, &viewport);
}

void AccelCommandBuffer::bindPipeline(const nmpt<const ComputePipeline> computePipeline_) {
    assert(false);
}

void AccelCommandBuffer::bindPipeline(const nmpt<const GraphicsPipeline> graphicsPipeline_) {

    const auto& graphicsPipeline = static_cast<cref<VkGraphicsPipeline>>(*graphicsPipeline_);
    _vkCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, reinterpret_cast<VkPipeline>(graphicsPipeline._vkPipe));
}
