#include "LORenderPass.hpp"
#include "../TextureFormat.hpp"
#include "../API/VkTranslate.hpp"
#include "../Framebuffer/Framebuffer.hpp"

using namespace ember::engine::gfx::pipeline;
using namespace ember::engine::gfx;
using namespace ember;

LORenderPass::LORenderPass(sptr<Device> device_) noexcept :
    _device(device_) {}

void LORenderPass::setup() {
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

        const auto hasColorRef { !colors.empty() };
        const auto hasDepthRef { !depthStencils.empty() };

        /**
         * Enforce Color Attachment Dependencies
         */
        if (hasColorRef) {
            _dependencies.push_back(
                vk::SubpassDependency {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::DependencyFlagBits::eByRegion
                }
            );

            _dependencies.push_back(
                vk::SubpassDependency {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::DependencyFlagBits::eByRegion
                }
            );
        }

        /**
         * Enforce Depth Attachment Dependencies
         */
        if (hasDepthRef) {
            _dependencies.push_back(
                vk::SubpassDependency {
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::DependencyFlagBits::eByRegion
                }
            );

            _dependencies.push_back(
                vk::SubpassDependency {
                    0,
                    VK_SUBPASS_EXTERNAL,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::DependencyFlagBits::eByRegion
                }
            );
        }
    }

    /**
     * Setup Info
     */
    vk::RenderPassCreateInfo info {
        vk::RenderPassCreateFlags(),
        static_cast<u32>(_attachments.size()),
        _attachments.data(),
        1ui32,
        &_subpass,
        static_cast<u32>(_dependencies.size()),
        _dependencies.data()
    };

    vk::RenderPassMultiviewCreateInfo mvi {};
    if (!_viewMasks.empty()) {
        mvi.subpassCount = _viewMasks.size();
        mvi.pViewMasks = _viewMasks.data();

        mvi.dependencyCount = 0;
        mvi.pViewOffsets = nullptr;

        mvi.correlationMaskCount = _correlationMasks.size();
        mvi.pCorrelationMasks = _correlationMasks.data();

        info.pNext = &mvi;
    }

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

void LORenderPass::setup(const Framebuffer& framebuffer_) {
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

        set(
            idx,
            vk::AttachmentDescription {
                vk::AttachmentDescriptionFlags(),
                api::vkTranslateFormat(entry->format()),
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                hasStencil ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare,
                hasStencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare,
                initialLayout,
                finalLayout
            }
        );
    }

    setup();
}

void LORenderPass::destroy() {
    _device->vkDevice().destroyRenderPass(_vkRenderPass);
}

void LORenderPass::set(const u32 idx_, const vk::AttachmentDescription& attachment_) {

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

const Vector<vk::AttachmentDescription>& LORenderPass::attachments() const noexcept {
    return _attachments;
}

vk::AttachmentDescription& LORenderPass::attachment(const u32 idx_) {
    return _attachments.at(idx_);
}

Vector<vk::SubpassDependency>& LORenderPass::dependencies() noexcept {
    return _dependencies;
}

cref<sptr<Device>> LORenderPass::device() const noexcept {
    return _device;
}

vk::SubpassDescription& LORenderPass::subpass() noexcept {
    return _subpass;
}

const vk::RenderPass& LORenderPass::vkRenderPass() const noexcept {
    return _vkRenderPass;
}

vk::RenderPass& LORenderPass::vkRenderPass() noexcept {
    return _vkRenderPass;
}

cref<vk::RenderPassBeginInfo> LORenderPass::vkBeginInfo() const noexcept {
    return _vkBeginInfo;
}
