#include "RenderTarget.hpp"

#include <Engine.GFX.Render/RenderPass.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Command/CommandBatch.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

RenderTarget::RenderTarget() :
    _device(nullptr),
    _renderer(nullptr),
    _renderPasses(),
    _swapchain(nullptr),
    _surface(nullptr),
    _otfImage(),
    _otfFinish(),
    _otfCpuGpu(),
    _enforceCpuGpuSync(true),
    _onTheFlight(true),
    _syncIdx(0ui32),
    _swapIdx(~0ui32),
    _swapSignal(),
    _active(false) {}

RenderTarget::~RenderTarget() {
    tidy();
}

void RenderTarget::tidy() {

    const auto req { _onTheFlight ? 2ui32 : 1ui32 };

    /**/

    for (u32 i = 0; i < req; ++i) {

        auto& renderPass = *_renderPasses[i];

        // Release bound Render Targets
        // renderPass.unbindTarget(<<symbol>>);
        auto storedSceneColor = renderPass.unbindTarget(makeSceneColorSymbol());
        assert(_swapchain->at(i) == storedSceneColor);

        // Release bound Scene View
        //renderPass.changeSceneView(clone(sceneView_));
        renderPass.unbindSceneView();
    }

    for (auto&& renderPass : _renderPasses) {
        _renderer->free(_STD move(renderPass));
    }
    _renderPasses.clear();

    /**
     *
     */
    for (u32 i = 0; i < _otfImage.size(); ++i) {
        _device->vkDevice().destroy(_otfImage[i]);
        _otfImage[i] = nullptr;

        //_device->vkDevice().destroy(_otfFinish[i]);
        //_otfFinish[i] = nullptr;

        _device->vkDevice().destroy(_otfCpuGpu[i]);
        _otfCpuGpu[i] = nullptr;
    }

    _otfImage.clear();
    //_otfFinish.clear();
    _otfCpuGpu.clear();
}

sptr<Device> RenderTarget::use(cref<sptr<Device>> device_) {
    return _STD exchange(_device, device_);
}

cref<sptr<Device>> RenderTarget::device() const noexcept {
    return _device;
}

nmpt<const Renderer> RenderTarget::use(mref<nmpt<const render::Renderer>> renderer_) {
    return _STD exchange(_renderer, renderer_);
}

non_owning_rptr<Swapchain> RenderTarget::use(cref<non_owning_rptr<Swapchain>> swapchain_) {

    assert(swapchain_ != nullptr);

    const auto req = _onTheFlight ? 1ui32 : 2ui32;
    if (swapchain_->imageCount() < req) {
        IM_CORE_WARNF(
            "Tried to use render target with unsupported swapchain. (min. Img. `{}`, provided `{}`)",
            req,
            swapchain_->imageCount()
        );
        return swapchain_;
    }

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

bool RenderTarget::ready() const noexcept {
    return _device && _renderer && _swapchain && !_renderPasses.empty();
}

bool RenderTarget::active() const noexcept {
    return _active;
}

bool RenderTarget::setActive(const bool active_) {
    const auto changed { _active != active_ };
    _active = active_;
    return changed;
}

void RenderTarget::buildPasses(mref<smr<const scene::SceneView>> sceneView_) {

    assert(_device);
    assert(_renderer);
    assert(_swapchain);

    const auto req { _onTheFlight ? 2ui32 : 1ui32 };

    /**/

    if (_renderer) {
        _renderPasses.reserve(req);

        for (u32 i = 0; i < req; ++i) {

            // TODO: Use contextual allocation
            _renderPasses.push_back(_renderer->allocate());
            auto& renderPass = *_renderPasses.back();

            // Store Scene View
            renderPass.changeSceneView(clone(sceneView_));

            // Store Render Targets
            // renderPass.bindTarget(<<symbol>>, <<texture>>);
            const auto bindResult = renderPass.bindTarget(makeSceneColorSymbol(), clone(_swapchain->at(i)));
            assert(bindResult);
        }
    }

    /**/

    if (_onTheFlight) {

        _otfImage.resize(req);
        //_otfFinish.resize(req);
        _otfCpuGpu.resize(req);

        const vk::SemaphoreCreateInfo sci {};
        const vk::FenceCreateInfo fci { vk::FenceCreateFlagBits::eSignaled };

        for (u32 i { 0ui32 }; i < req; ++i) {

            _otfImage[i] = _device->vkDevice().createSemaphore(sci);
            //_otfFinish[i] = _device->vkDevice().createSemaphore(sci);
            _otfCpuGpu[i] = _device->vkDevice().createFence(fci);

            assert(_otfImage[i]);
            //assert(_otfFinish[i]);
            assert(_otfCpuGpu[i]);
        }

    }
}

bool RenderTarget::rebuildPasses(cref<non_owning_rptr<Swapchain>> nextSwapChain_) {

    /**
     *
     */
    const auto req { _onTheFlight ? 2ui32 : 1ui32 };

    /**
     * Block render passes
     */
    if (_enforceCpuGpuSync) {

        for (u32 i = 0; i < req; ++i) {

            auto cpuGpuSync { _renderPasses[i]->unsafeSync() };
            if (cpuGpuSync) {
                auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
                // Might fail when one was submitted or pipeline gets dumped/error state
                assert(result == vk::Result::eSuccess);
            }
        }
    }

    /**/

    const auto prevSwapChain = use(nextSwapChain_);
    if (prevSwapChain == nextSwapChain_ /* failed */) {
        return false;
    }

    /**
     *
     */
    for (u32 i = 0; i < req; ++i) {

        auto renderPass = _STD move(_renderPasses[i]);

        // Update Scene View
        // renderPass->changeSceneView(sceneView_); // Not needed, cause same scene view

        // Update Render Targets
        // renderPass.bindTarget(<<symbol>>, <<texture>>);
        const auto previous = renderPass->unbindTarget(makeSceneColorSymbol());
        const auto bindResult = renderPass->bindTarget(makeSceneColorSymbol(), clone(_swapchain->at(i)));
        assert(bindResult);

        _renderPasses[i] = _renderer->reallocate(_STD move(renderPass)).first;
        assert(_renderPasses[i]);
    }

    /**/
    return true;
}

nmpt<RenderPass> RenderTarget::next() {
    /**
     * Check for enforced sync behaviour
     */
    if (_enforceCpuGpuSync) {
        auto cpuGpuSync { _renderPasses[_syncIdx]->unsafeSync() };

        if (cpuGpuSync) {
            auto result { _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX) };
            assert(result == vk::Result::eSuccess);
        }
    }

    /**
     * Next Swapchain Image
     */
    s64 nextIdx { ~0ui32 };
    smr<Texture> nextImage {};
    vk::Semaphore nextSignal {};

    const auto nextResult { _swapchain->acquireNext(nextIdx, nextImage, nextSignal) };

    /**
     * Ensure swapchain complies our expectations
     */
    if (not nextResult) {
        return nullptr;
    }

    if (!_onTheFlight) {
        /**
         * Get Invocation / Await Last Render
         */
        const auto& pass = _renderPasses[_syncIdx];
        if (!pass->await()) {
            IM_CORE_WARN("Failed to validate await state of render invocation.");
        }
    }

    _swapSignal = nextSignal;
    _swapIdx = nextIdx;

    /**
     *
     */
    return _renderPasses[nextIdx].get();
}

void RenderTarget::update() {

    auto& renderPass = _renderPasses[_syncIdx];

    if (_onTheFlight && _swapSignal) {
        renderPass->clearTargetWaitSignals(makeSceneColorSymbol());
        renderPass->addTargetWaitSignal(makeSceneColorSymbol(), _swapSignal);
    }

    if (_onTheFlight) {
        /*
        renderPass->getTargetReadySignals().clear();
        renderPass->getTargetReadySignals().push_back(_otfFinish[_syncIdx]);
        */
    }

    auto updated = _renderer->update(_STD move(renderPass));
    (*updated)();
    _renderPasses[_syncIdx] = _STD move(updated);
}

RenderEnqueueResult RenderTarget::finish(
    cref<Vector<vk::Semaphore>> waits_/* , cref<Vector<vk::Semaphore>> signals_ */
) {
    /**
     * Combine Semaphores
     */
    Vector<vk::Semaphore> waits {};
    waits.insert(waits.end(), waits_.begin(), waits_.end());

    if (_onTheFlight) {
        //waits.push_back(_otfFinish[_syncIdx]);
        _renderPasses[_syncIdx]->enumerateTargetReadySignals(waits);
    }

    /*
    Vector<vk::Semaphore> signals {};
    signals.insert(signals.end(), signals_.begin(), signals_.end());
     */

    /**
     *
     */
    const auto vkResult { _swapchain->presentNext(_swapIdx, waits) };
    RenderEnqueueResult result {};

    if (vkResult == vk::Result::eSuboptimalKHR || vkResult == vk::Result::eErrorOutOfDateKHR) {
        result = RenderEnqueueResult::eFailedChanged;
    } else if (vkResult != vk::Result::eSuccess) {
        result = RenderEnqueueResult::eFailed;
    }

    /**
     *
     */
    nextSync();
    return result;
}
