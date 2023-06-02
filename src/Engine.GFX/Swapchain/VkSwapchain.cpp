#include "VkSwapchain.hpp"

#include <Engine.Scheduler/Fiber/Fiber.hpp>

#include "../Texture/TextureFactory.hpp"

using namespace hg::engine::gfx;
using namespace hg;

VkSwapchain::VkSwapchain() :
    Swapchain(),
    _rQueue(3ui32),
    _pQueue(3ui32),
    _desiredImages(2ui32) {}

VkSwapchain::~VkSwapchain() {
    destroy();
}

void VkSwapchain::setup(cref<sptr<Device>> device_) {
    /**/
    Swapchain::setup(device_);

    /**/
    assert(_device);

    assert(_extent.x > 0ui32 && _extent.y > 0ui32);
    assert(_format != TextureFormat::eUndefined);
    assert(_desiredImages > 0ui32 && _desiredImages <= 2ui32);

    /**/
    auto* factory { TextureFactory::get() };

    const bool isDepth { isDepthFormat(_format) };
    const bool isStencil { isStencilFormat(_format) };

    vk::ImageAspectFlags iaf {};
    vk::ImageUsageFlags iuf {
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc
    };

    if (isDepth && not isStencil) {
        iaf |= vk::ImageAspectFlagBits::eDepth;
        iuf |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

    } else if (not isDepth && isStencil) {
        iaf |= vk::ImageAspectFlagBits::eStencil;
        iuf |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

    } else if (isDepth && isStencil) {
        iaf |= vk::ImageAspectFlagBits::eDepth;
        iaf |= vk::ImageAspectFlagBits::eStencil;
        iuf |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

    } else {
        iaf |= vk::ImageAspectFlagBits::eColor;
        iuf |= vk::ImageUsageFlagBits::eColorAttachment;
    }

    const TextureBuildPayload payload {
        math::uivec3 { _extent.x, _extent.y, 1 },
        _format,
        1ui32,
        // TODO: Check whether we want isolated images or texture array
        TextureType::e2d,
        iaf,
        iuf,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::SharingMode::eExclusive
    };

    _images.resize(_desiredImages);
    for (u32 i { 0ui32 }; i < _desiredImages; ++i) {

        /**/
        auto instance { factory->build(payload) };
        factory->buildView(instance);

        /**/
        _images[i].image = make_sptr<Texture>(_STD move(instance));

        /**/
        assert(_images[i].image);
        assert(_images[i].image->vkView());
        assert(_images[i].image->buffer().image());

        /**/
        #ifdef _DEBUG
        assert(_rQueue.try_push(i));
        #else
        _rQueue.try_push(i);
        #endif
    }
}

void VkSwapchain::destroy() {

    /**
     *
     */
    for (auto& entry : _images) {

        /**/
        entry.image->destroy();

        /**/
        if (entry.readySignal) {
            _device->vkDevice().destroySemaphore(entry.readySignal);
            entry.readySignal = VK_NULL_HANDLE;
        }
    }

    _images.clear();

    /**/
    u64 tmp {};
    while (_rQueue.try_pop(tmp)) {}
    while (_pQueue.try_pop(tmp)) {}
}

bool VkSwapchain::acquireNext(ref<s64> idx_, ref<sptr<Texture>> image_, ref<vk::Semaphore> signal_) {

    u64 ridx { ~0ui64 };
    /*
    while (!_rQueue.try_pop(qidx)) {
        scheduler::fiber::self::yield();
    }
     */
    if (!_rQueue.try_pop(ridx)) {
        return false;
    }

    /**
     *
     */
    _images[ridx].presentWaits.clear();

    /**
     * Resolve
     */
    idx_ = ridx;
    image_ = _images[ridx].image;
    signal_ = (_images[ridx].readySignal) ? _images[ridx].readySignal : VK_NULL_HANDLE;

    return true;
}

vk::Result VkSwapchain::presentNext(u64 idx_) {

    ref<SwapchainImage> image { _images[idx_] };

    u64 tmp { idx_ };
    if (!_pQueue.try_push(_STD move(tmp))) {
        return vk::Result::eErrorUnknown;
    }

    return vk::Result::eSuccess;
}

vk::Result VkSwapchain::presentNext(u64 idx_, cref<Vector<vk::Semaphore>> waits_) {

    ref<SwapchainImage> image { _images[idx_] };

    /**/

    image.presentWaits.insert(
        image.presentWaits.begin(),
        waits_.begin(),
        waits_.end()
    );

    /**/

    u64 tmp { idx_ };
    if (!_pQueue.try_push(_STD move(tmp))) {
        return vk::Result::eErrorUnknown;
    }

    return vk::Result::eSuccess;
}

bool VkSwapchain::consumeNext(
    ref<sptr<Texture>> image_,
    ref<vk::Semaphore> signal_,
    ref<Vector<vk::Semaphore>> waits_
) {

    u64 pidx { ~0ui64 };
    if (!_pQueue.try_pop(pidx)) {
        return false;
    }

    /**
     * Ensure signal
     */
    if (not _images[pidx].readySignal) {
        _images[pidx].readySignal = nextSignal();
    }

    /**
     * Resolve
     */
    image_ = _images[pidx].image;
    signal_ = _images[pidx].readySignal;
    waits_ = _images[pidx].presentWaits;

    /**
     * Rebind
     */
    #ifdef _DEBUG
    if (not _rQueue.try_push(_STD move(pidx))) {
        // Warning: We must ensure that image index is pushed to ready queue to prevent resource leak
        __debugbreak();
    }
    #else
    while (not _rQueue.try_push(_STD move(pidx))) {
        scheduler::fiber::self::yield();
    }
    #endif

    return true;
}

void VkSwapchain::setExtent(cref<math::uivec2> extent_) {
    _extent = extent_;
}

void VkSwapchain::setFormat(TextureFormat format_) {
    _format = format_;
}

s64 VkSwapchain::imageCount() const noexcept {
    return _images.size();
}

void VkSwapchain::setDesiredImages(const u32 desired_) {
    _desiredImages = desired_;
}

vk::Semaphore VkSwapchain::nextSignal() {
    const vk::SemaphoreCreateInfo info {};
    const auto signal { _device->vkDevice().createSemaphore(info) };
    return signal;
}
