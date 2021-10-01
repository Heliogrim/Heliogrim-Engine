#include "VkSwapchain.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../API/VkTranslate.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VkSwapchain::VkSwapchain(cref<sptr<Device>> device_, cref<Surface> surface_) :
    Swapchain(),
    _device(device_),
    _surface(surface_) {}

VkSwapchain::~VkSwapchain() {
    VkSwapchain::destroy();
}

void VkSwapchain::setup() {

    SCOPED_STOPWATCH

    assert(_device);
    assert(/*_surface*/true);

    const auto capabilities = _device->vkPhysicalDevice().getSurfaceCapabilitiesKHR(_surface);
    const auto modes = _device->vkPhysicalDevice().getSurfacePresentModesKHR(_surface);

    _extent = clampExtent({ 1920, 1080 }, capabilities);
    const auto mode = selectPresentMode(modes);

    _format = _surface.getImageFormat(*_device);

    vk::SurfaceTransformFlagBitsKHR surfaceTransform;
    if (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        surfaceTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        surfaceTransform = capabilities.currentTransform;
    }

    u32 length = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && length > capabilities.maxImageCount) {
        length = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR sci {
        vk::SwapchainCreateFlagsKHR(),
        _surface,
        length,
        api::vkTranslateFormat(_format),
        vk::ColorSpaceKHR::eSrgbNonlinear,
        vk::Extent2D { _extent.x, _extent.y },
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        surfaceTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        mode,
        VK_TRUE,
        nullptr
    };

    if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc) {
        sci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst) {
        sci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
    }

    _vkSwapchain = _device->vkDevice().createSwapchainKHR(sci);
    assert(_vkSwapchain);

    const auto swapImages = _device->vkDevice().getSwapchainImagesKHR(_vkSwapchain);
    _images.resize(swapImages.size(), nullptr);

    for (u32 i = 0; i < swapImages.size(); ++i) {
        /**
         *
         */
        _images[i] = make_sptr<Texture>();
        auto& texture = _images[i];

        assert(texture);

        /**
         *
         */
        texture->type() = TextureType::e2d;
        texture->format() = _format;
        texture->extent() = { _extent, 1ui32 };
        texture->mipLevels() = 1ui32;
        texture->buffer()._vkAspect = vk::ImageAspectFlagBits::eColor;
        texture->buffer().image() = swapImages[i];

        TextureFactory::get()->buildView(*texture);

        assert(texture->vkView());
        assert(texture->buffer().image());
    }
}

void VkSwapchain::destroy() {

    SCOPED_STOPWATCH

    for (auto& entry : _images) {
        entry->destroy();
    }

    _images.clear();

    _device->vkDevice().destroySwapchainKHR(_vkSwapchain);
    _vkSwapchain = nullptr;
}

cref<vk::SwapchainKHR> VkSwapchain::vkSwapchain() const noexcept {
    return _vkSwapchain;
}

math::uivec2 VkSwapchain::clampExtent(math::uivec2 extent_, cref<vk::SurfaceCapabilitiesKHR> capabilities_) noexcept {
    if (capabilities_.currentExtent.width == 0xFFFFFFFF) {

        if (extent_.x < capabilities_.minImageExtent.width) {
            extent_.x = capabilities_.minImageExtent.width;
        } else if (extent_.x > capabilities_.maxImageExtent.width) {
            extent_.x = capabilities_.maxImageExtent.width;
        }

        if (extent_.y < capabilities_.minImageExtent.height) {
            extent_.y = capabilities_.minImageExtent.height;
        } else if (extent_.x > capabilities_.maxImageExtent.height) {
            extent_.y = capabilities_.maxImageExtent.height;
        }

    } else {
        extent_ = {
            capabilities_.currentExtent.width,
            capabilities_.currentExtent.height
        };
    }

    return extent_;
}

vk::PresentModeKHR VkSwapchain::selectPresentMode(cref<vector<vk::PresentModeKHR>> modes_) const noexcept {
    vk::PresentModeKHR pm { vk::PresentModeKHR::eFifo };

    for (u32 i = 0; i < modes_.size(); ++i) {
        if (modes_[i] == vk::PresentModeKHR::eMailbox) {
            pm = vk::PresentModeKHR::eMailbox;
            break;
        }

        if (modes_[i] == vk::PresentModeKHR::eImmediate) {
            pm = vk::PresentModeKHR::eImmediate;
        }
    }

    return pm;
}
