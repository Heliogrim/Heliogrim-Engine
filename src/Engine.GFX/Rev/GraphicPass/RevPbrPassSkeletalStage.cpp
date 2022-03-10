#include "RevPbrPassSkeletalStage.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevPbrPass.hpp"
#include "../../Graphics.hpp"
#include "../../VkFixedPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevPbrPassSkeletalStage::RevPbrPassSkeletalStage(ptr<RevPbrPass> graphicPass_) :
    GraphicPassPipelineStage(),
    _graphicPass(graphicPass_) {}

void RevPbrPassSkeletalStage::setup() {

    SCOPED_STOPWATCH

    auto device = _graphicPass->device();

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

    _pipeline->vertexStage().shaderSlot().name() = "skeletal_pbr";
    _pipeline->fragmentStage().shaderSlot().name() = "skeletal_pbr";

    _pipeline->rasterizationStage().cullFace() = RasterCullFace::eBack;

    /**
     *
     */
    _pipeline->setup();
}

void RevPbrPassSkeletalStage::destroy() noexcept {

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

void RevPbrPassSkeletalStage::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {}

void RevPbrPassSkeletalStage::freeWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {}

bool RevPbrPassSkeletalStage::check(ptr<const ProcessedModelBatch> batch_) noexcept {
    return batch_ != nullptr;
}

void RevPbrPassSkeletalStage::before(cref<GraphicPassStageContext> ctx_) {

    SCOPED_STOPWATCH

    /**
     * Prepare Command Buffer
     */
    _cmd->begin();

    const auto entry { ctx_.state.data.at("RevPbrPass::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    _cmd->beginRenderPass(*_renderPass, frame);
    _cmd->bindPipeline(_pipeline.get(), {
        frame.width(),
        frame.height(),
        0.F,
        1.F
    });
}

void RevPbrPassSkeletalStage::process(cref<GraphicPassStageContext> ctx_, ptr<const ProcessedModelBatch> batch_) {

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

void RevPbrPassSkeletalStage::after(cref<GraphicPassStageContext> ctx_) {

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

sptr<FixedPipeline> RevPbrPassSkeletalStage::pipeline() const noexcept {
    return _pipeline;
}
