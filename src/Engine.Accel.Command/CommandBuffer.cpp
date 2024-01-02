#include "CommandBuffer.hpp"

#include <Engine.Accel.Pass/VkGraphicsPass.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>

using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

AccelCommandBuffer::AccelCommandBuffer(mref<CommandBuffer> commandBuffer_) noexcept :
    CommandBuffer(_STD move(commandBuffer_)) {}

AccelCommandBuffer::AccelCommandBuffer(mref<this_type> other_) noexcept :
    CommandBuffer(_STD move(other_)) {}

AccelCommandBuffer::~AccelCommandBuffer() noexcept {
    if (_vkCmd) {
        release();
    }
}

void AccelCommandBuffer::commit() {
    assert(_valid);
    _valid = not _faf;
}

void AccelCommandBuffer::bindComputePass(const nmpt<const ComputePass> computePass_) {
    assert(false);
}

void AccelCommandBuffer::bindRenderPass(mref<BindRenderPassStruct> bindRenderPass_) {

    const auto& graphicsPass = static_cast<cref<VkGraphicsPass>>(bindRenderPass_.graphicsPass);

    /**/

    ptr<const Vector<vk::ClearValue>> clearValues = &bindRenderPass_.clearValues;

    Vector<vk::ClearValue> tmpClearValues {};
    if (bindRenderPass_.clearValues.empty()) {

        tmpClearValues.reserve(graphicsPass._references.size());
        clearValues = &tmpClearValues;

        for (const auto& reference : graphicsPass._references) {
            if (
                reference.layout == vk::ImageLayout::eDepthAttachmentOptimal ||
                reference.layout == vk::ImageLayout::eDepthStencilAttachmentOptimal
            ) {
                tmpClearValues.emplace_back(vk::ClearDepthStencilValue { 1.F, 0uL });
            } else {
                tmpClearValues.emplace_back(vk::ClearColorValue { 0.F, 0.F, 0.F, 1.F });
            }
        }
    }

    assert(clearValues->size() == graphicsPass._references.size());

    /**/

    auto renderArea = vk::Rect2D {
        vk::Offset2D {}, vk::Extent2D { bindRenderPass_.framebuffer.width(), bindRenderPass_.framebuffer.height() }
    };
    vk::RenderPassBeginInfo rpbi {
        reinterpret_cast<VkRenderPass>(graphicsPass._vkGraphicsPass),
        bindRenderPass_.framebuffer.vkFramebuffer(),
        renderArea,
        static_cast<u32>(clearValues->size()),
        clearValues->data()
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

    _pipelineBindPoint = vk::PipelineBindPoint::eCompute;
    _pipelineLayout = nullptr;

    assert(false);
}

void AccelCommandBuffer::bindPipeline(const nmpt<const GraphicsPipeline> graphicsPipeline_) {

    const auto& graphicsPipeline = static_cast<cref<VkGraphicsPipeline>>(*graphicsPipeline_);
    _vkCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, reinterpret_cast<VkPipeline>(graphicsPipeline._vkPipe));

    _pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    _pipelineLayout = reinterpret_cast<const vk::PipelineLayout&>(graphicsPipeline._vkPipeLayout);
}
