#include "PbrPassStaticStage.hpp"

#include "PbrPass.hpp"
#include "../VkFixedPipeline.hpp"
#include "../Graphics.hpp"

using namespace ember::engine::gfx;
using namespace ember;

PbrPassStaticStage::PbrPassStaticStage(ptr<PbrPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void PbrPassStaticStage::setup() {
    sptr<Device> device = Graphics::get()->getCurrentDevice();

    /**
     * Command Buffer
     */
    _cmd = make_sptr<CommandBuffer>(_STD move(device->graphicsQueue()->pool()->make()));

    /**
     * Render Pass
     */
    _renderPass = make_sptr<pipeline::RenderPass>(device);
    assert(_renderPass);

    _renderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    });

    _renderPass->set(1, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    });

    _renderPass->set(2, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR8G8B8A8Unorm,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    });

    _renderPass->set(3, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eD32SfloatS8Uint,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    });

    /**
     *
     */
    _renderPass->setup();

    /**
     * Fixed Pipeline
     */
    _pipeline = make_sptr<VkFixedPipeline>(device, _renderPass);
    assert(_pipeline);

    _pipeline->topology() = PrimitiveTopology::eTriangleList;
    _pipeline->viewport() = Viewport {};

    _pipeline->vertexStage().shaderSlot().name() = "static_pbr";
    _pipeline->fragmentStage().shaderSlot().name() = "static_pbr";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;

    /**
     *
     */
    _pipeline->setup();
}

void PbrPassStaticStage::destroy() noexcept {
    /**
     *
     */
    if (_pipeline) {
        _pipeline->destroy();
        _pipeline.reset();
    }

    /**
     *
     */
    if (_renderPass) {
        _renderPass->destroy();
        _renderPass.reset();
    }

    /**
     *
     */
    if (_cmd && _cmd->vkCommandBuffer()) {
        _cmd->release();
    }
}

bool PbrPassStaticStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ != nullptr;
}

void PbrPassStaticStage::before(cref<GraphicPassStageContext> ctx_) {
    /**
     * Prepare Command Buffer
     */
    _cmd->begin();
    _cmd->beginRenderPass(*_renderPass, _graphicPass->currentFramebuffer());
    _cmd->bindPipeline(_pipeline.get());
}

void PbrPassStaticStage::process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) {
    if (batch_->empty()) {
        return;
    }

    // TODO: cmd.bindDescriptor(...);
    _cmd->bindVertexBuffer(0, batch_->geometry().vertices, 0);
    _cmd->bindVertexBuffer(1, batch_->geometry().instances, 0);
    _cmd->bindIndexBuffer(batch_->geometry().indices, 0);

    for (const auto& entry : batch_->executions()) {

        cref<DistinctBind> db = entry.bind;

        cref<DistinctGeometry> dg = entry.geometry;
        _cmd->drawIndexed(dg.instanceCount, dg.instanceOffset, dg.indexCount, dg.indexOffset, 0ui32);
    }
}

void PbrPassStaticStage::after(cref<GraphicPassStageContext> ctx_) {
    /**
     * End Command Buffer
     */
    _cmd->endRenderPass();
    _cmd->end();

    /**
     * Submit Command Buffer to CommandBatch
     */
    ctx_.batch.push(*_cmd);
}

sptr<pipeline::RenderPass> PbrPassStaticStage::renderPass() const noexcept {
    return _renderPass;
}

sptr<FixedPipeline> PbrPassStaticStage::pipeline() const noexcept {
    return _pipeline;
}
