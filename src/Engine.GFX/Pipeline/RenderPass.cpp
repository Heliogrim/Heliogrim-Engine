#include "RenderPass.hpp"
#include "../TextureFormat.hpp"
#include "../API/VkTranslate.hpp"
#include "../Framebuffer/Framebuffer.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

RenderPass::RenderPass(sptr<Device> device_) noexcept :
    _device(device_) {}

void RenderPass::setup() {
    /**
     * Prepare
     */
    Vector<vk::AttachmentReference> colors {};
    Vector<vk::AttachmentReference> depthStencils {};
    Vector<vk::AttachmentReference> preserves {};

    _clearValues.clear();

    for (const auto& entry : _references) {

        switch (entry.layout) {
            case vk::ImageLayout::eColorAttachmentOptimal: {
                colors.push_back(entry);
                _clearValues.push_back(vk::ClearColorValue { _STD array<float, 4> { 0.F, 0.F, 0.F, 0.F } });
                break;
            }

            case vk::ImageLayout::eDepthAttachmentOptimal:
            case vk::ImageLayout::eStencilAttachmentOptimal:
            case vk::ImageLayout::eDepthStencilAttachmentOptimal: {
                depthStencils.push_back(entry);
                _clearValues.push_back(vk::ClearDepthStencilValue { 1.F, 0 });
                break;
            }
        }
    }

    _subpass = {
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        static_cast<u32>(colors.size()),
        colors.data(),
        nullptr,
        depthStencils.data(),
        0,
        nullptr
    };

    /**
     * TODO:
     * Warning: Determine Subpass Dependencies
     */
    if (_dependencies.empty()) {
        _dependencies.push_back(vk::SubpassDependency {
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            vk::DependencyFlagBits::eByRegion
        });

        _dependencies.push_back(vk::SubpassDependency {
            0,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            vk::AccessFlagBits::eShaderRead,
            vk::DependencyFlagBits::eByRegion
        });
    }

    /**
     * Setup Info
     */
    const vk::RenderPassCreateInfo info {
        vk::RenderPassCreateFlags(),
        static_cast<u32>(_attachments.size()),
        _attachments.data(),
        1ui32,
        &_subpass,
        static_cast<u32>(_dependencies.size()),
        _dependencies.data()
    };

    /**
     * Create
     */
    _vkRenderPass = _device->vkDevice().createRenderPass(info);

    /**
     * Prepare Begin Info
     */
    _vkBeginInfo = vk::RenderPassBeginInfo {
        _vkRenderPass,
        nullptr,
        {},
        static_cast<u32>(_clearValues.size()),
        _clearValues.data()
    };
}

void RenderPass::setup(const Framebuffer& framebuffer_) {
    const auto& attachments = framebuffer_.attachments();

    /**
     * We need a index mapped set of attachments so render pass
     *	and framebuffer match attachments by idx
     */
    for (u32 idx = 0; idx < attachments.size(); ++idx) {

        const auto& entry = attachments[idx];

        const auto hasDepth = isDepthFormat(entry.unwrapped()->format());
        const auto hasStencil = isStencilFormat(entry.unwrapped()->format());

        const auto hasColor = !hasDepth && !hasStencil;

        vk::ImageLayout initialLayout {};
        vk::ImageLayout finalLayout {};

        if (hasColor) {
            initialLayout = vk::ImageLayout::eUndefined;
            finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        } else if (hasDepth && !hasStencil) {
            initialLayout = vk::ImageLayout::eUndefined;
            finalLayout = vk::ImageLayout::eDepthAttachmentOptimal;

        } else if (!hasDepth && hasStencil) {
            initialLayout = vk::ImageLayout::eUndefined;
            finalLayout = vk::ImageLayout::eStencilAttachmentOptimal;

        } else if (hasDepth && hasStencil) {
            initialLayout = vk::ImageLayout::eUndefined;
            finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        }

        if ((hasDepth || hasStencil) && /* Warning: If separate depth and stencil is unsupported */true) {
            initialLayout = vk::ImageLayout::eUndefined;
            finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        /**
         * Check for SwapImage FrameBuffer
         */
        if (entry->buffer()._vkLayout == vk::ImageLayout::ePresentSrcKHR) {
            finalLayout = vk::ImageLayout::ePresentSrcKHR;
        }

        // TODO: Check whether we can assume the dependencies based on the framebuffer attachments
        // TODO: I don't think so, cause dependencies are propergated by pipeline usage like G-Buffer and Shadow Pass, but we can actually try

        set(idx, vk::AttachmentDescription {
            vk::AttachmentDescriptionFlags(),
            api::vkTranslateFormat(entry->format()),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            hasStencil ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare,
            hasStencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
            initialLayout,
            finalLayout
        });
    }

    setup();
}

void RenderPass::destroy() {
    _device->vkDevice().destroyRenderPass(_vkRenderPass);
}

void RenderPass::set(const u32 idx_, const vk::AttachmentDescription& attachment_) {

    const TextureFormat format { api::vkTranslateFormat(attachment_.format) };
    const auto hasDepth = isDepthFormat(format);
    const auto hasStencil = isStencilFormat(format);

    /**
     * Check whether list of attachment does already contains idx_
     */
    if (idx_ < _attachments.size()) {
        // Store Attachment
        _attachments[idx_] = attachment_;

        // Store Reference
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;

        if (hasDepth && hasStencil) {
            layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        } else if (hasDepth) {
            layout = vk::ImageLayout::eDepthAttachmentOptimal;
        } else if (hasStencil) {
            layout = vk::ImageLayout::eStencilAttachmentOptimal;
        } else {
            layout = vk::ImageLayout::eColorAttachmentOptimal;
        }

        if ((hasDepth || hasStencil) && /* Warning: If separate depth and stencil is unsupported */true) {
            layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        _references[idx_] = vk::AttachmentReference {
            idx_,
            layout
        };

        return;
    }

    /**
     * Resize to contain idx_
     */
    _attachments.resize(idx_ + 1ui32);
    _references.resize(idx_ + 1ui32);

    /**
     * Recursive Call
     */
    set(idx_, attachment_);
}

const Vector<vk::AttachmentDescription>& RenderPass::attachments() const noexcept {
    return _attachments;
}

vk::AttachmentDescription& RenderPass::attachment(const u32 idx_) {
    return _attachments.at(idx_);
}

Vector<vk::SubpassDependency>& RenderPass::dependencies() noexcept {
    return _dependencies;
}

cref<sptr<Device>> RenderPass::device() const noexcept {
    return _device;
}

vk::SubpassDescription& RenderPass::subpass() noexcept {
    return _subpass;
}

const vk::RenderPass& RenderPass::vkRenderPass() const noexcept {
    return _vkRenderPass;
}

vk::RenderPass& RenderPass::vkRenderPass() noexcept {
    return _vkRenderPass;
}

cref<vk::RenderPassBeginInfo> RenderPass::vkBeginInfo() const noexcept {
    return _vkBeginInfo;
}
