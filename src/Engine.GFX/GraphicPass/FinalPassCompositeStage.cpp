#include "FinalPassCompositeStage.hpp"

#include <cassert>
#include <Engine.Common/Make.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "FinalPass.hpp"
#include "../PrimitiveTopology.hpp"
#include "../RasterCullFace.hpp"
#include "../VkFixedPipeline.hpp"
#include "../Graphics.hpp"
#include "../API/VkTranslate.hpp"

using namespace ember::engine::gfx;
using namespace ember;

FinalPassCompositeStage::FinalPassCompositeStage(ptr<FinalPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void FinalPassCompositeStage::setup() {

    SCOPED_STOPWATCH

    auto device = Graphics::get()->getCurrentDevice();
    const auto* swapchain = Graphics::get()->getCurrentSwapchain();

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
        api::vkTranslateFormat(swapchain->format()),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
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

    _pipeline->vertexStage().shaderSlot().name() = "finalpass";
    _pipeline->fragmentStage().shaderSlot().name() = "finalpass";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eNone;
    _pipeline->rasterizationStage().depthCheck() = false;

    static_cast<ptr<VkFixedPipeline>>(_pipeline.get())->blending().push_back(vk::PipelineColorBlendAttachmentState {
        VK_FALSE,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    });

    /**
     *
     */
    _pipeline->setup();
}

void FinalPassCompositeStage::destroy() noexcept {

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

bool FinalPassCompositeStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ == nullptr;
}

void FinalPassCompositeStage::before(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

    /**
     * Prepare Command Buffer
     */
    _cmd->begin();
    _cmd->beginRenderPass(*_renderPass, _graphicPass->currentFramebuffer());
    _cmd->bindPipeline(_pipeline.get());
}

void FinalPassCompositeStage::process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) {

    SCOPED_STOPWATCH

    // TODO: cmd.bindDescriptor(...);
    _cmd->draw(1, 0, 6, 0);
}

void FinalPassCompositeStage::after(cref<GraphicPassStageContext> ctx_) {

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

sptr<pipeline::RenderPass> FinalPassCompositeStage::renderPass() const noexcept {
    return _renderPass;
}

sptr<FixedPipeline> FinalPassCompositeStage::pipeline() const noexcept {
    return _pipeline;
}
