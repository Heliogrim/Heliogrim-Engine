#include "RevDepthSharedNode.hpp"

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

#include <Engine.GFX.Glow/Texture/RevVirtualMarkerTexture.hpp>

#include "__macro.hpp"

using namespace ember::engine::gfx::glow::render;
using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

#pragma region Global Shared Marker
/**/

wptr<glow::RevVirtualMarkerTexture> globalMarkerTexture {};

/**/
#pragma endregion

RevDepthSharedNode::RevDepthSharedNode() :
    RenderStageNode(),
    _device(nullptr),
    _loRenderPass(nullptr) {}

RevDepthSharedNode::~RevDepthSharedNode() {
    destroyLORenderPass();
}

void RevDepthSharedNode::setup(cref<sptr<Device>> device_) {

    // Store Device
    _device = device_;

    // LORenderPass
    setupLORenderPass();
}

void RevDepthSharedNode::destroy() {

    // LORenderPass
    destroyLORenderPass();
}

bool RevDepthSharedNode::allocate(const ptr<HORenderPass> renderPass_) {
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
    auto depth = factory->build({
        buffer.extent(),
        REV_DEPTH_FORMAT,
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eDepth,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    auto marker = factory->build({
        buffer.extent(),
        REV_EARLY_STREAM_MARKER_FORMAT,
        1ui32,
        TextureType::e2d,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    });

    factory->buildView(depth);
    factory->buildView(marker);

    /**
     * Create Framebuffer :: Attach
     */
    buffer.add({ _STD move(depth) });
    buffer.add({ _STD move(marker) });

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
            "RevDepthStage::CommandBuffer"sv,
            make_sptr<decltype(cmd)>(_STD move(cmd))
        ).second
    );
    assert(
        renderPass_->state()->data.insert_or_assign(
            "RevDepthStage::Framebuffer"sv,
            make_sptr<decltype(buffer)>(_STD move(buffer))
        ).second
    );
    #else
    renderPass_->state()->data.insert_or_assign("RevDepthStage::CommandBuffer"sv,
        make_sptr<decltype(cmd)>(_STD move(cmd)));
    renderPass_->state()->data.insert_or_assign("RevDepthStage::Framebuffer"sv,
        make_sptr<decltype(buffer)>(_STD move(buffer)));
    #endif

    /**
     * Post Process
     */
    postProcessAllocated(renderPass_);

    /**/
    auto markerTexture { globalMarkerTexture.lock() };
    if (!markerTexture) {
        markerTexture = make_sptr<RevVirtualMarkerTexture>();
        markerTexture->setup();

        globalMarkerTexture = markerTexture;
    }
    /**/

    renderPass_->state()->data.insert_or_assign("RevDepthStage::MarkerTexture"sv, markerTexture);

    //
    return true;
}

bool RevDepthSharedNode::free(const ptr<HORenderPass> renderPass_) {

    auto it { renderPass_->state()->data.find("RevDepthStage::MarkerTexture"sv) };
    if (it != renderPass_->state()->data.end()) {
        sptr<RevVirtualMarkerTexture> markerTexture {
            _STD static_pointer_cast<RevVirtualMarkerTexture, void>(it->second)
        };

        renderPass_->state()->data.erase(it);
    }

    /**
     *
     */
    it = renderPass_->state()->data.find("RevDepthStage::Framebuffer"sv);
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
    it = renderPass_->state()->data.find("RevDepthStage::CommandBuffer"sv);
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

Vector<RenderDataToken> RevDepthSharedNode::requiredToken() noexcept {
    return {};
}

Vector<RenderDataToken> RevDepthSharedNode::optionalToken() noexcept {
    return {};
}

void RevDepthSharedNode::before(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    RenderStageNode::before(renderPass_, stagePass_);

    const auto& data { renderPass_->state()->data };

    /**
     * Prepare Command Buffer
     */
    const auto cmdEntry { data.at("RevDepthStage::CommandBuffer"sv) };
    auto& cmd { *_STD static_pointer_cast<CommandBuffer, void>(cmdEntry) };

    cmd.begin();

    /**
     * Begin LORenderPass :: Sub-Pass Content
     */
    const auto frameEntry { data.at("RevDepthStage::Framebuffer"sv) };
    auto& framebuffer { *_STD static_pointer_cast<Framebuffer, void>(frameEntry) };

    cmd.beginRenderPass(*_loRenderPass, framebuffer, false);
}

void RevDepthSharedNode::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_,
    const non_owning_rptr<SceneNodeModel> model_
) const {
    RenderStageNode::invoke(renderPass_, stagePass_, model_);
}

void RevDepthSharedNode::after(
    const non_owning_rptr<HORenderPass> renderPass_,
    const non_owning_rptr<RenderStagePass> stagePass_
) const {
    RenderStageNode::after(renderPass_, stagePass_);

    const auto& data { renderPass_->state()->data };

    /**
     * Get Command Buffer
     */
    const auto cmdEntry { data.at("RevDepthStage::CommandBuffer"sv) };
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

cref<sptr<pipeline::LORenderPass>> RevDepthSharedNode::loRenderPass() const noexcept {
    return _loRenderPass;
}

void RevDepthSharedNode::setupLORenderPass() {

    /**
     * LORenderPass
     */
    _loRenderPass = make_sptr<pipeline::LORenderPass>(_device);

    // Depth Attachment
    _loRenderPass->set(0, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        api::vkTranslateFormat(REV_DEPTH_FORMAT),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    });

    _loRenderPass->set(1, vk::AttachmentDescription {
        vk::AttachmentDescriptionFlags(),
        api::vkTranslateFormat(REV_EARLY_STREAM_MARKER_FORMAT),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eShaderReadOnlyOptimal
    });

    /**
     *
     */
    _loRenderPass->setup();
}

void RevDepthSharedNode::destroyLORenderPass() {

    if (_loRenderPass) {
        _loRenderPass->destroy();
        _loRenderPass.reset();
    }
}

void RevDepthSharedNode::postProcessAllocated(const ptr<HORenderPass> renderPass_) {

    const auto it { renderPass_->state()->data.find("RevDepthStage::Framebuffer"sv) };
    if (it == renderPass_->state()->data.end()) {
        return;
    }

    const ptr<Framebuffer> buffer {
        static_cast<const ptr<Framebuffer>>(it->second.get())
    };

    Vector<vk::ImageMemoryBarrier> imgBarriers {};
    for (const auto& entry : buffer->attachments()) {

        auto& attachment { *entry.unwrapped() };

        if (!isDepthFormat(attachment.format()) && !isStencilFormat(attachment.format())) {
            continue;
        }

        imgBarriers.push_back({
            vk::AccessFlags {},
            vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            attachment.buffer().image(),
            vk::ImageSubresourceRange {
                (isDepthFormat(attachment.format()) ?
                     vk::ImageAspectFlagBits::eDepth :
                     vk::ImageAspectFlagBits::eNoneKHR) |
                (isStencilFormat(attachment.format()) ?
                     vk::ImageAspectFlagBits::eStencil :
                     vk::ImageAspectFlagBits::eNoneKHR),
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
        vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
        vk::DependencyFlags {},
        0, nullptr,
        0, nullptr,
        static_cast<u32>(imgBarriers.size()), imgBarriers.data()
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();
}
