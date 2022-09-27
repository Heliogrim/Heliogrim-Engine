#include "RenderTarget.hpp"

#include "Command/CommandBatch.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/HORenderPass.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

RenderTarget::RenderTarget() :
    _device(nullptr),
    _renderer(nullptr),
    _passes(),
    _swapchain(nullptr),
    _surface(nullptr),
    _otfImage(),
    _otfFinish(),
    _otfCpuGpu(),
    _enforceCpuGpuSync(true),
    _onTheFlight(true),
    _syncIdx(0ui32) {}

RenderTarget::~RenderTarget() {
    tidy();
}

void RenderTarget::tidy() {

    const auto req { _onTheFlight ? 2ui32 : 1ui32 };

    /**
     *
     */
    for (auto* entry : _passes) {
        _renderer->free(_STD move(entry));
    }

    _passes.clear();

    /**
     *
     */
    for (u32 i = 0; i < _otfImage.size(); ++i) {
        _device->vkDevice().destroy(_otfImage[i]);
        _otfImage[i] = nullptr;

        _device->vkDevice().destroy(_otfFinish[i]);
        _otfFinish[i] = nullptr;

        _device->vkDevice().destroy(_otfCpuGpu[i]);
        _otfCpuGpu[i] = nullptr;
    }

    _otfImage.clear();
    _otfFinish.clear();
    _otfCpuGpu.clear();
}

sptr<Device> RenderTarget::use(cref<sptr<Device>> device_) {
    return _STD exchange(_device, device_);
}

cref<sptr<Device>> RenderTarget::device() const noexcept {
    return _device;
}

non_owning_rptr<render::Renderer> RenderTarget::use(cref<non_owning_rptr<render::Renderer>> renderer_) {
    return _STD exchange(_renderer, renderer_);
}

non_owning_rptr<Swapchain> RenderTarget::use(cref<non_owning_rptr<Swapchain>> swapchain_) {
    return _STD exchange(_swapchain, swapchain_);
}

u32 RenderTarget::supportedFramesAhead() const noexcept {
    if (_onTheFlight) {
        // Use 2 Sync Sets -> (1 Current + 1 Ahead) | (1 Previous + 1 Current)
        return 1ui32;
    }

    return 0ui32;
}

non_owning_rptr<Surface> RenderTarget::use(cref<non_owning_rptr<Surface>> surface_) {
    return _STD exchange(_surface, surface_);
}

void RenderTarget::nextSync() {
    // Fast flip-flip idx
    _syncIdx = 1ui32 - _syncIdx;
}

void RenderTarget::rebuildPasses(cref<ptr<Camera>> camera_, cref<ptr<scene::IRenderScene>> scene_) {

    assert(_device);
    assert(_renderer);
    assert(_swapchain);

    const auto req { _onTheFlight ? 2ui32 : 1ui32 };

    /**
     *
     */
    _passes.reserve(req);

    for (u32 i { 0ui32 }; i < req; ++i) {
        _passes.push_back(_renderer->allocate({
            _swapchain->at(i),
            scene_,
            camera_
        }));
    }

    /**
     *
     */
    if (_onTheFlight) {

        _otfImage.resize(req);
        _otfFinish.resize(req);
        _otfCpuGpu.resize(req);

        const vk::SemaphoreCreateInfo sci {};
        const vk::FenceCreateInfo fci { vk::FenceCreateFlagBits::eSignaled };

        for (u32 i { 0ui32 }; i < req; ++i) {

            _otfImage[i] = _device->vkDevice().createSemaphore(sci);
            _otfFinish[i] = _device->vkDevice().createSemaphore(sci);
            _otfCpuGpu[i] = _device->vkDevice().createFence(fci);

            assert(_otfImage[i]);
            assert(_otfFinish[i]);
            assert(_otfCpuGpu[i]);
        }

    }

}

const non_owning_rptr<render::HORenderPass> RenderTarget::next() {

    /**
     * Check for enforced sync behaviour
     */
    if (_enforceCpuGpuSync) {

        auto cpuGpuSync { _passes[_syncIdx]->unsafeSync() };

        if (cpuGpuSync) {
            auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
            assert(result == vk::Result::eSuccess);
        }

    }

    /**
     * Next Swapchain Image
     */
    u32 image { ~0ui32 };
    auto nextResult {
        _device->vkDevice().acquireNextImageKHR(
            static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
            UINT64_MAX,
            _onTheFlight ? _otfImage[_syncIdx] : VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            &image
        )
    };

    _swapchain->setCurrentIdx(image);

    if (!_onTheFlight) {
        /**
         * Get Invocation / Await Last Render
         */
        auto* pass { _passes[_syncIdx] };

        if (!pass->await()) {
            DEBUG_SNMSG(false, "WARN", "Failed to validate await state of render invocation.")
        }
    }

    /**
     *
     */
    return _passes[image];
}

void RenderTarget::update() {

    /**
     * Render
     */
    CommandBatch layout {};

    if (_onTheFlight) {
        layout.pushBarrier(_otfImage[_syncIdx]);
        layout.pushSignal(_otfFinish[_syncIdx]);

        layout.barrierStages() = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }

    auto* pass { _passes[_syncIdx] };
    const auto* feedback { _renderer->invoke(pass, layout) };
    assert(feedback);
}

void RenderTarget::finish(cref<Vector<vk::Semaphore>> waits_/* , cref<Vector<vk::Semaphore>> signals_ */) {

    /**
     * If we got no surface, we can't present results
     */
    if (!_surface) {
        nextSync();
        return;
    }

    /**
     * Combine Semaphores
     */
    Vector<vk::Semaphore> waits {};
    waits.insert(waits.end(), waits_.begin(), waits_.end());

    if (_onTheFlight) {
        waits.push_back(_otfFinish[_syncIdx]);
    }

    /*
    Vector<vk::Semaphore> signals {};
    signals.insert(signals.end(), signals_.begin(), signals_.end());
     */

    /**
     *
     */
    const auto index { _swapchain->currentIdx() };

    const vk::PresentInfoKHR info {
        static_cast<u32>(waits.size()),
        waits.data(),
        1ui32,
        &(static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain()),
        &index
    };

    auto result { _device->graphicsQueue()->vkQueue().presentKHR(info) };
    assert(result == vk::Result::eSuccess);

    /**
     *
     */
    nextSync();
}
