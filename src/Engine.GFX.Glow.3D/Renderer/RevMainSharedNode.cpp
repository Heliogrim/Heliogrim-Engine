#include "RevMainSharedNode.hpp"

#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Command/CommandPool.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX/Renderer/HORenderPass.hpp>
#include <Engine.GFX/Renderer/RenderDataToken.hpp>
#include <Engine.GFX/Renderer/RenderPassState.hpp>
#include <Engine.GFX/Renderer/RenderStagePass.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>

#include "__macro.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

RevMainSharedNode::RevMainSharedNode() :
    RenderStageNode(),
    _device(nullptr),
    _loRenderPass(nullptr) {}

RevMainSharedNode::~RevMainSharedNode() {
    destroyLORenderPass();
}

void RevMainSharedNode::setup(cref<sptr<Device>> device_) {

    // Store Device
    _device = device_;

    // LORenderPass
    setupLORenderPass();
}

void RevMainSharedNode::destroy() {

    // LORenderPass
    destroyLORenderPass();
}

bool RevMainSharedNode::allocate(const ptr<HORenderPass> renderPass_) {

    /**
     * Allocate Command Buffer
     */
    auto cmd { _device->graphicsQueue()->pool()->make() };

    /**
     * Create Framebuffer
     */
    const auto* factory { TextureFactory::get() };

    Framebuffer buffer { _device };
    buffer.setExtent(renderPass_->target()->extent());
    buffer.setRenderPass(_loRenderPass);

    /**
     * Create Framebuffer :: Attachments
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
     * Query Depth-Attachment of `early z-pass`
     */
    const auto depthEntry { renderPass_->state()->data.at("RevDepthStage::Framebuffer"sv) };
    auto& depthFramebuffer { *_STD static_pointer_cast<Framebuffer, void>(depthEntry) };

    const auto& depthAttachment { depthFramebuffer.attachments()[0] };

    /**
     * Create Framebuffer :: Attach
     */
    buffer.add({ _STD move(albedo) });
    buffer.add({ _STD move(normal) });
    buffer.add({ _STD move(position) });
    buffer.add({ _STD move(mrs) });
    buffer.add(depthAttachment);

    /**
     *
     */
    buffer.setup();

    /**
     * Store State
     */
    #if _DEBUG
    assert(
        renderPass_->state()->data.insert_or_assign(
            "RevMainStage::CommandBuffer"sv,
            make_sptr<decltype(cmd)>(_STD move(cmd))
        ).second
    );
    assert(
        renderPass_->state()->data.insert_or_assign(
            "RevMainStage::Framebuffer"sv,
            make_sptr<decltype(buffer)>(_STD move(buffer))
        ).second
    );
    #else
    renderPass_->state()->data.insert_or_assign("RevMainStage::CommandBuffer"sv,
        make_sptr<decltype(cmd)>(_STD move(cmd)));
    renderPass_->state()->data.insert_or_assign("RevMainStage::Framebuffer"sv,
        make_sptr<decltype(buffer)>(_STD move(buffer)));
    #endif

    /**
     * Post Process
     */
    postProcessAllocated(renderPass_);

    //
    return true;
}

bool RevMainSharedNode::free(const ptr<HORenderPass> renderPass_) {

    /**
     *
     */
    auto it { renderPass_->state()->data.find("RevMainStage::Framebuffer"sv) };
    if (it != renderPass_->state()->data.end()) {

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
        renderPass_->state()->data.erase(it);
    }

    /**
     * Free Command Buffers
     */
    it = renderPass_->state()->data.find("RevMainStage::CommandBuffer"sv);
    if (it != renderPass_->state()->data.end()) {

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
        renderPass_->state()->data.erase(it);
    }

    //
    return true;
}

Vector<RenderDataToken> RevMainSharedNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevMainSharedNode::optionalToken() noexcept {
    return {};
}

void RevMainSharedNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    RenderStageNode::before(renderPass_, stagePass_);

    const auto& data { renderPass_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevMainStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    cmd.begin();

    /**
     * Begin LORenderPass :: Sub-Pass Content
     */
    const auto frameEntry { data.at("RevMainStage::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(frameEntry) };

    cmd.beginRenderPass(*_loRenderPass, framebuffer, false);
}

void RevMainSharedNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {
    RenderStageNode::invoke(renderPass_, stagePass_, model_);
}

void RevMainSharedNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    RenderStageNode::after(renderPass_, stagePass_);

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevMainStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    /**
     * End LORenderPass :: Sub-Pass Content
     */
    cmd.endRenderPass();

    /**
     * End Command Buffer
     */
    cmd.end();

    /**
     * Submit Command Buffer
     */
    stagePass_->batch().push(cmd);
}

cref<sptr<pipeline::LORenderPass>> RevMainSharedNode::loRenderPass() const noexcept {
    return _loRenderPass;
}

void RevMainSharedNode::setupLORenderPass() {

    /**
     * LORenderPass
     */
    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Albedo Attachment :: Used to store surface albedo color
    _loRenderPass->set(0, vk::AttachmentDescription {
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
    _loRenderPass->set(1, vk::AttachmentDescription {
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
    _loRenderPass->set(2, vk::AttachmentDescription {
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
    _loRenderPass->set(3, vk::AttachmentDescription {
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
    _loRenderPass->set(4, vk::AttachmentDescription {
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
    _loRenderPass->setup();
}

void RevMainSharedNode::destroyLORenderPass() {

    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

void RevMainSharedNode::postProcessAllocated(const ptr<HORenderPass> renderPass_) const {

    const auto it { renderPass_->state()->data.find("RevMainStage::Framebuffer"sv) };
    if (it == renderPass_->state()->data.end()) {
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
