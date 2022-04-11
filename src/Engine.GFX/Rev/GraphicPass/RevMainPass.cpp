#if FALSE

#include "RevMainPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevDepthPass.hpp"
#include "RevMainPassSkeletalStage.hpp"
#include "RevMainPassSkyStage.hpp"
#include "RevMainPassStaticStage.hpp"
#include "../../Graphics.hpp"
#include "../../Texture/TextureFactory.hpp"
#include "__macro.hpp"
#include "../../API/VkTranslate.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevMainPass::RevMainPass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::eMainPass),
    _processor(this) {}

cref<sptr<pipeline::ApiRenderPass>> RevMainPass::renderPass() const noexcept {
    return _renderPass;
}

void RevMainPass::setupRenderPass() {

    /**
     * Render Pass
     */
    _renderPass = make_sptr<pipeline::ApiRenderPass>(_device);
    assert(_renderPass);

    // Albedo Attachment :: Used to store surface albedo color
    _renderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR8G8B8A8Unorm,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        //vk::ImageLayout::eColorAttachmentOptimal
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    // Normal Attachment :: Used to store surface normals
    _renderPass->set(1, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        //vk::ImageLayout::eColorAttachmentOptimal
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    // Position Attachment :: Used to store surface position
    _renderPass->set(2, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR32G32B32A32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        //vk::ImageLayout::eColorAttachmentOptimal
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    // Mrs Attachment :: Used to store mrs data (metalness, roughness, specular)
    _renderPass->set(3, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        vk::Format::eR16G16B16A16Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        //vk::ImageLayout::eColorAttachmentOptimal
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    // Depth Attachment :: Pre enriched depth buffer attachment
    _renderPass->set(4, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        api::vkTranslateFormat(REV_DEPTH_FORMAT),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eLoad,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    });

    /**
     *
     */
    _renderPass->setup();
}

void RevMainPass::setup() {

    SCOPED_STOPWATCH

    //
    setupRenderPass();

    /**
     *
     */
    auto skyStage = new RevMainPassSkyStage { this };
    skyStage->setup();
    _pipeline.add(skyStage);

    auto staticStage = new RevMainPassStaticStage { this };
    staticStage->setup();
    _pipeline.add(staticStage);

    //auto skeletalStage = new RevMainPassSkeletalStage { this };
    //skeletalStage->setup();
    //_pipeline.add(skeletalStage);
}

void RevMainPass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();

    /**
     *
     */
    if (_renderPass) {
        _renderPass->destroy();
        _renderPass.reset();
    }
}

void RevMainPass::postProcessAllocated(const ptr<RenderPassState> state_) const {

    const auto it { state_->data.find("RevMainPass::Framebuffer"sv) };
    if (it == state_->data.end()) {
        return;
    }

    const ptr<Framebuffer> buffer {
        static_cast<const ptr<Framebuffer>>(it->second.get())
    };

    Vector<vk::ImageMemoryBarrier> imgBarriers {};
    for (const auto& entry : buffer->attachments()) {

        auto& attachment { *entry.unwrapped() };

        if (isDepthFormat(attachment.format()) || isStencilFormat(attachment.format())) {
            continue;
        }

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            attachment.buffer().image(),
            vk::ImageSubresourceRange {
                vk::ImageAspectFlagBits::eColor,
                0,
                attachment.mipLevels(),
                0,
                attachment.layer()
            }

        });
    }

    auto pool = _device->graphicsQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**
     * Transform
     */
    cmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
        0, nullptr,
        0, nullptr,
        static_cast<u32>(imgBarriers.size()), imgBarriers.data()
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
}

void RevMainPass::allocateWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) {

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     *
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer buffer { _device };
    buffer.setExtent(invocation_->target()->extent());
    buffer.setRenderPass(_renderPass);

    /**
     *
     */
    auto albedo = factory->build({
        buffer.extent(),
        TextureFormat::eR8G8B8A8Unorm,
        // Surface Albedo
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    auto normal = factory->build({
        buffer.extent(),
        TextureFormat::eR32G32B32A32Sfloat,
        // Surface Normal
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    auto position = factory->build({
        buffer.extent(),
        TextureFormat::eR32G32B32A32Sfloat,
        // Surface Position
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    auto mrs = factory->build({
        buffer.extent(),
        TextureFormat::eR16G16B16A16Sfloat,
        // Mrs data Sampling
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    factory->buildView(albedo);
    factory->buildView(normal);
    factory->buildView(position);
    factory->buildView(mrs);

    /**
     *
     */
    const auto depthEntry { state_->data.at("RevDepthPass::Framebuffer"sv) };
    auto& depthFrame { *_STD static_pointer_cast<Framebuffer, void>(depthEntry) };

    cref<FramebufferAttachment> depth = depthFrame.attachments()[0];

    /**
     *
     */
    buffer.add(FramebufferAttachment { _STD move(albedo) });
    buffer.add(FramebufferAttachment { _STD move(normal) });
    buffer.add(FramebufferAttachment { _STD move(position) });
    buffer.add(FramebufferAttachment { _STD move(mrs) });
    buffer.add(depth);

    /**
     *
     */
    buffer.setup();

    /**
     * Store Resources
     */
    state_->data.insert_or_assign("RevMainPass::CommandBuffer"sv, _STD make_shared<decltype(cmd)>(_STD move(cmd)));
    state_->data.insert_or_assign("RevMainPass::Framebuffer"sv, _STD make_shared<decltype(buffer)>(_STD move(buffer)));

    /**
     *
     */
    GraphicPass::allocateWith(invocation_, state_);

    /**
     * Post Process Images
     */
    postProcessAllocated(state_);
}

void RevMainPass::freeWith(const ptr<const RenderPass> invocation_, const ptr<RenderPassState> state_) {
    GraphicPass::freeWith(invocation_, state_);

    /**
     *
     */
    auto it { state_->data.find("RevMainPass::Framebuffer"sv) };
    if (it != state_->data.end()) {

        sptr<Framebuffer> buffer {
            _STD static_pointer_cast<Framebuffer, void>(it->second)
        };

        /**
         *
         */
        buffer->destroy();

        for (auto& entry : buffer->attachments()) {
            entry->destroy();
        }

        /**
         *
         */
        state_->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = state_->data.find("RevMainPass::CommandBuffer"sv);
    if (it != state_->data.end()) {

        sptr<CommandBuffer> cmd {
            _STD static_pointer_cast<CommandBuffer, void>(it->second)
        };

        /**
         *
         */
        if (cmd->vkCommandBuffer()) {
            auto& lck { cmd->lck() };
            lck.acquire();
            cmd->release();
            lck.release();
        }

        /**
         *
         */
        state_->data.erase(it);
    }
}

void RevMainPass::process(const ptr<scene::RenderGraph> graph_, const ptr<const RenderContext> ctx_,
    ref<CommandBatch> batch_) {

    const auto& data { ctx_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevMainPass::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };
    cmd.begin();

    const auto entry { ctx_->state()->data.at("RevMainPass::Framebuffer"sv) };
    auto& frame { *_STD static_pointer_cast<Framebuffer, void>(entry) };

    /**
     * Begin ApiRenderPass :: Subpass Content
     */
    cmd.beginRenderPass(*_renderPass, frame);

    // TODO:
    {
        GraphicPassStageContext stageCtx { batch_ };
        pipeline().stages()[0]->before(ctx_, stageCtx);
        pipeline().stages()[0]->process(ctx_, stageCtx, nullptr);
        pipeline().stages()[0]->after(ctx_, stageCtx);
    }

    /**
     *
     */
    GraphicPass::process(graph_, ctx_, batch_);

    /**
     * End ApiRenderPass :: Subpass Content
     */
    cmd.endRenderPass();

    /**
     * End Command Buffer
     */
    cmd.end();

    /**
     * Submit Command Buffer to CommandBatch
     */
    batch_.push(cmd);
}

ptr<GraphicPassModelProcessor> RevMainPass::processor() noexcept {
    _processor.reset();
    return &_processor;
}


#endif
