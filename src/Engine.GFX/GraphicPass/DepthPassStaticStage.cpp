#include "DepthPassStaticStage.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "DepthPass.hpp"
#include "../Graphics.hpp"
#include "../VkFixedPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

DepthPassStaticStage::DepthPassStaticStage(ptr<DepthPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void DepthPassStaticStage::setup() {

    SCOPED_STOPWATCH

    auto device = _graphicPass->device();
    const auto* swapchain = _graphicPass->swapchain();

    /**
     * Command Buffer
     */
    // TODO: Check whether we need one command buffer per swapchain image like 'On the flight cmd'
    _cmd = make_sptr<CommandBuffer>(_STD move(device->graphicsQueue()->pool()->make()));

    /**
     * Render Pass
     */
    _renderPass = make_sptr<pipeline::RenderPass>(device);
    assert(_renderPass);

    _renderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eD32SfloatS8Uint,
        vk::SampleCountFlagBits::e1,
        // Warning: Only first element at framebuffer have to clear attachment memory to reset
        vk::AttachmentLoadOp::eClear,
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
    _pipeline->viewport() = Viewport {
        swapchain->width(),
        swapchain->height(),
        0.F,
        1.F
    };

    _pipeline->inputs().push_back(FixedPipelineInput {
        0ui8,
        InputRate::ePerVertex,
        sizeof(vertex),
        vector<InputAttribute> {
            { 0ui32, TextureFormat::eR32G32B32Sfloat, offsetof(vertex, position) },
            { 1ui32, TextureFormat::eR8G8B8Unorm, offsetof(vertex, color) },
            { 2ui32, TextureFormat::eR32G32B32Sfloat, offsetof(vertex, uvm) },
            { 3ui32, TextureFormat::eR32G32B32Sfloat, offsetof(vertex, normal) }
        }
    });

    _pipeline->vertexStage().shaderSlot().name() = "depthpass_static";
    _pipeline->fragmentStage().shaderSlot().name() = "depthpass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;

    /**
     *
     */
    _pipeline->setup();
}

void DepthPassStaticStage::destroy() noexcept {

    SCOPED_STOPWATCH

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

bool DepthPassStaticStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ != nullptr;
}

void DepthPassStaticStage::before(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

    /**
     * Prepare Command Buffer
     */
    _cmd->begin();
    _cmd->beginRenderPass(*_renderPass, _graphicPass->currentFramebuffer());
    _cmd->bindPipeline(_pipeline.get());
}

void DepthPassStaticStage::process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) {

    SCOPED_STOPWATCH

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

void DepthPassStaticStage::after(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

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

sptr<pipeline::RenderPass> DepthPassStaticStage::renderPass() const noexcept {
    return _renderPass;
}

sptr<FixedPipeline> DepthPassStaticStage::pipeline() const noexcept {
    return _pipeline;
}
