#include "Graphics.hpp"

#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "todo.h"
#include "Command/CommandBatch.hpp"
#include "Engine.Common/Cast.hpp"
#include "GraphicPass/DepthPass.hpp"
#include "GraphicPass/FinalPass.hpp"
#include "GraphicPass/LightPass.hpp"
#include "GraphicPass/PbrPass.hpp"
#include "GraphicPass/ProbePass.hpp"
#include "Loader/TextureLoader.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"
#include "Texture/VkTextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember::engine;
using namespace ember;

Graphics::Graphics(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _swapchain(nullptr),
    _computeQueue(nullptr),
    _graphicsQueue(nullptr),
    _transferQueue(nullptr),
    _renderScene(nullptr),
    _uiRenderScene(nullptr) {}

Graphics::~Graphics() {
    tidy();
}

void Graphics::setup() {

    SCOPED_STOPWATCH

    /**
     * Create a new application
     */
    _application = {};
    _application.setup();

    /**
     * Create a new Surface (aka. Window)
     */
    _surface = { _session, &_application };
    _surface.setup();

    /**
     * Prepare rendering specific data structures
     *
     *	a: create a new device to access graphics architecture  
     */
    _device = make_sptr<Device>(_application, &_surface);
    _device->setup();

    _computeQueue = _device->computeQueue();
    _graphicsQueue = _device->graphicsQueue();
    _transferQueue = _device->transferQueue();

    /**
     * Create Utilities
     */
    VkTextureFactory::make(_device);
    loader::TextureLoader::make(_device);

    /**
     * Create a new swapchain to present image
     */
    _swapchain = new VkSwapchain(_device, _surface);
    _swapchain->setup();

    /**
     * Create Shader Storage
     */
    ShaderStorage::make();
    build_shader(_device);

    /**
     * Create Graphic Passes
     */
    _graphicPasses.resize(static_cast<u32>(GraphicPassMask::eFinalPass) + 1, nullptr);

    ptr<DepthPass> depthPass;

    _graphicPasses[static_cast<u8>(GraphicPassMask::eDepthPass)] = (depthPass = new DepthPass(_device, _swapchain));
    _graphicPasses[static_cast<u8>(GraphicPassMask::eLightPass)] = new LightPass(_device, _swapchain);
    _graphicPasses[static_cast<u8>(GraphicPassMask::eProbePass)] = new ProbePass(_device, _swapchain);
    //_graphicPasses[static_cast<u8>(GraphicPassMask::ePbrPass)] = new PbrPass(_device, _swapchain, depthPass);
    _graphicPasses[static_cast<u8>(GraphicPassMask::ePbrPass)] = new ProbePass(_device, _swapchain);
    _graphicPasses[static_cast<u8>(GraphicPassMask::eFinalPass)] = new FinalPass(_device, _swapchain);

    /**
     *
     */
    for (auto& entry : _graphicPasses) {
        entry->setup();
    }

    /**
     * Prepare Synchronization of Passes
     */
    {
        constexpr GraphicPassMask masks[] {
            GraphicPassMask::eDepthPass,
            GraphicPassMask::eLightPass,
            GraphicPassMask::eProbePass,
            GraphicPassMask::ePbrPass
        };
        constexpr u8 length = static_cast<u8>(sizeof(masks)) - 1;

        _graphicPassFences.resize(length);

        for (u8 idx = 0; idx < length; ++idx) {
            _graphicPassFences[idx] = _device->vkDevice().createFence({});
        }
    }

    /**
     *
     */
    _graphicPassBatches.resize(static_cast<u32>(GraphicPassMask::eFinalPass) + 1ui32, {});

    /**
     * Prepare On Flight Synchronization
     */
    {
        _onFlightSync.image.resize(_swapchain->length());
        _onFlightSync.finish.resize(_swapchain->length());
        _onFlightSync.cpuGpuSync.resize(_swapchain->length());

        const vk::SemaphoreCreateInfo si {};
        const vk::FenceCreateInfo fi {};

        for (u32 idx = 0; idx < _swapchain->length(); ++idx) {
            _onFlightSync.image[idx] = _device->vkDevice().createSemaphore(si);
            _onFlightSync.finish[idx] = _device->vkDevice().createSemaphore(si);
            _onFlightSync.cpuGpuSync[idx] = _device->vkDevice().createFence(fi);

            #ifdef _DEBUG
            assert(_onFlightSync.image[idx]);
            assert(_onFlightSync.finish[idx]);
            assert(_onFlightSync.cpuGpuSync[idx]);
            #endif

            /**
             * Submit Fence to prevent error on first frame
             */
            _graphicsQueue->submit(CommandBatch {}, _onFlightSync.cpuGpuSync[idx]);
        }
    }

    _scheduled.store(0);
}

void Graphics::schedule() {

    u8 expect = { 0 };
    if (!_scheduled.compare_exchange_strong(expect, 1)) {
        throw _STD runtime_error("Tried to schedule already running graphics handle.");
    }

    reschedule();
}

void Graphics::tidy() {

    SCOPED_STOPWATCH

    /**
     * Wait until schedule stopped execution
     */
    u8 expect { 1 };
    _scheduled.compare_exchange_strong(expect, 2);

    expect = 0;
    while (_scheduled.load() != expect) {
        scheduler::thread::self::yield();
    }

    /**
     * Wait until graphics queues stopped execution
     */
    _device->computeQueue()->vkQueue().waitIdle();
    _device->graphicsQueue()->vkQueue().waitIdle();
    _device->transferQueue()->vkQueue().waitIdle();

    _computeQueue->destroy();
    _graphicsQueue->destroy();
    _transferQueue->destroy();

    /**
     *
     */
    for (auto& entry : _graphicPassFences) {
        _device->vkDevice().destroyFence(entry);
    }

    for (auto& entry : _graphicPasses) {
        entry->destroy();
    }

    _graphicPasses.clear();

    /**
     *
     */
    for (u32 idx = 0; idx < _swapchain->length(); ++idx) {
        _device->vkDevice().destroy(_onFlightSync.image[idx]);
        _onFlightSync.image[idx] = nullptr;

        _device->vkDevice().destroy(_onFlightSync.finish[idx]);
        _onFlightSync.finish[idx] = nullptr;

        _device->vkDevice().destroy(_onFlightSync.cpuGpuSync[idx]);
        _onFlightSync.cpuGpuSync[idx] = nullptr;
    }
    _onFlightSync.image.clear();
    _onFlightSync.finish.clear();
    _onFlightSync.cpuGpuSync.clear();

    /**
     * Swapchain
     */
    delete _swapchain;
    _swapchain = nullptr;

    ShaderStorage::destroy();

    /**
     * Device
     */
    _device->destroy();
    _device.reset();

    _surface.destroy();
    _application.destroy();
}

sptr<Session> Graphics::session() const noexcept {
    return _session;
}

sptr<Device> Graphics::getCurrentDevice() const noexcept {
    return _device;
}

ptr<Swapchain> Graphics::getCurrentSwapchain() const noexcept {
    return _swapchain;
}

CommandQueue::reference_type Graphics::getComputeQueue() const noexcept {
    return *_device->computeQueue();
}

CommandQueue::reference_type Graphics::getGraphicsQueue() const noexcept {
    return *_device->graphicsQueue();
}

CommandQueue::reference_type Graphics::getTransferQueue() const noexcept {
    return *_device->transferQueue();
}

static u64 frame = 0;

void Graphics::_tick(ptr<scene::IRenderScene> scene_) {

    SCOPED_STOPWATCH

    /**
     * 
     */
    static u32 syncIdx = 0;

    // TODO: Make timeout as small as possible to skip blocking request -> if failed, return an reschedule tick task
    #ifdef _DEBUG
    assert(
        _device->vkDevice().waitForFences(1, &_onFlightSync.cpuGpuSync[syncIdx], VK_TRUE, UINT64_MAX) ==
        vk::Result::eSuccess
    );
    #else
    _device->vkDevice().waitForFences(1, &_onFlightSync.cpuGpuSync[syncIdx], VK_TRUE, UINT64_MAX);
    #endif
    _device->vkDevice().resetFences(1, &_onFlightSync.cpuGpuSync[syncIdx]);

    /**
     * 
     */
    u32 swapIdx;
    [[maybe_unused]] auto result = _device->vkDevice().acquireNextImageKHR(
        static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
        UINT64_MAX,
        _onFlightSync.image[syncIdx],
        nullptr,
        &swapIdx
    );

    _swapchain->setCurrentIdx(swapIdx);

    /**
     *
     */
    processGraphicPasses(scene_);

    /**
     * 
     */
    auto& batch = _graphicPassBatches[static_cast<u8>(GraphicPassMask::eFinalPass)];
    auto finalPass = _graphicPasses[static_cast<u8>(GraphicPassMask::eFinalPass)];

    batch.reset();
    finalPass->process(scene_->renderGraph(), batch);

    /**
     * Execute Command Batch
     */

    batch.pushBarrier(_onFlightSync.image[syncIdx]);
    batch.pushSignal(_onFlightSync.finish[syncIdx]);
    batch.barrierStages() = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    _graphicsQueue->submit(batch, _onFlightSync.cpuGpuSync[syncIdx]);

    /**
     * Display Image
     */
    const vk::PresentInfoKHR present {
        1,
        &_onFlightSync.finish[syncIdx],
        1,
        &static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
        &swapIdx
    };

    #ifdef _DEBUG
    assert(_graphicsQueue->vkQueue().presentKHR(present) == vk::Result::eSuccess);
    #else
    _graphicsQueue->vkQueue().presentKHR(present);
    #endif

    /**
     * Flip-Flop Sync Idx
     */
    syncIdx = 1 - syncIdx;

    /**
     * Debug Metrics
     */
    static uint64_t frames = 0;
    static auto last = std::chrono::high_resolution_clock::now();

    const auto now = _STD chrono::high_resolution_clock::now();
    if (_STD chrono::duration_cast<_STD chrono::milliseconds>(now - last).count() > 1000) {
        _STD cout << "Frames: " << frames << _STD endl;

        frames = 0;
        last = now;
    }

    ++frames;
}

void Graphics::processGraphicPasses(ptr<scene::IRenderScene> scene_) {

    SCOPED_STOPWATCH

    constexpr GraphicPassMask masks[] {
        GraphicPassMask::eDepthPass,
        GraphicPassMask::eLightPass,
        GraphicPassMask::eProbePass,
        GraphicPassMask::ePbrPass
    };
    constexpr u8 length = static_cast<u8>(sizeof(masks)) - 1;

    /**
     * Reset synchronization for all processed passes
     */
    [[maybe_unused]] auto resetResult = _device->vkDevice().resetFences(static_cast<u32>(_graphicPassFences.size()),
        _graphicPassFences.data());

    for (u8 idx = 0; idx < length; ++idx) {
        /**
         *
         */
        GraphicPassMask mask = masks[idx];

        /**
         *
         */
        auto& batch = _graphicPassBatches[static_cast<u8>(mask)];
        auto& graphicPass = _graphicPasses[static_cast<u8>(mask)];

        batch.reset();

        /**
         * Wait for Dependencies
         */
        Vector<GraphicPassMask> dependencies {};

        if (dependencies.size() >= 1) {
            // TODO: get back dependencies for RenderPass finish
            Vector<vk::Fence> waitFences {};

            waitFences.reserve(dependencies.size());
            for (const auto& mask : dependencies) {
                waitFences.push_back(_graphicPassFences[static_cast<u8>(mask)]);
            }

            [[maybe_unused]] auto result = _device->vkDevice().waitForFences(waitFences.size(), waitFences.data(),
                VK_TRUE, UINT64_MAX);
        }

        /**
         * Invoke and Store GraphicsPass
         */
        graphicPass->process(scene_->renderGraph(), batch);

        /**
         * Submit Batch to Queue
         */
        _graphicsQueue->submit(batch, _graphicPassFences[idx]);
    }

    /**
     * Sync all passes for final computation
     */
    [[maybe_unused]] auto waitAllResult = _device->vkDevice().waitForFences(_graphicPassFences.size(),
        _graphicPassFences.data(), VK_TRUE, UINT64_MAX);
}

void Graphics::reschedule() {

    /**
     * Guard already stopped scheduling
     */
    if (_scheduled.load(_STD memory_order_relaxed) != 1ui8) {
        return;
    }

    scheduler::exec(
        make_repetitive_task([this]() {

                u8 expect { 1 };
                if (_scheduled.load(_STD memory_order_relaxed) == expect) {

                    /**
                     * TODO: Replace Guard
                     */
                    if (_renderScene) {
                        _tick(_renderScene);
                    }

                    return true;
                }

                expect = { 2 };
                _scheduled.compare_exchange_strong(expect, 0);
                return false;

            },
            scheduler::task::TaskMask::eCritical,
            scheduler::ScheduleStageBarriers::eGraphicNodeCollectWeak,
            scheduler::ScheduleStageBarriers::eBottomStrong
        )
    );

}

#include <Ember/StaticGeometryComponent.hpp>
#include <Engine.Scene/RevScene.hpp>
#include "Scene/StaticGeometryModel.hpp"
#include "Scene/SceneTag.hpp"

bool Graphics::useAsRenderScene(const ptr<scene::IRenderScene> scene_) {
    _renderScene = scene_;

    if (scene_ == nullptr) {
        return true;
    }

    // TODO: Move callback register to other possition and reconcider lifecycle \
    //  cause elements added to scene before callback register will be lost data

    /**
     *
     */
    auto* const scene { static_cast<const ptr<scene::RevScene>>(scene_) };
    scene->setNodeType(GfxSceneTag {}, StaticGeometryComponent::type_id, [](const ptr<SceneComponent> owner_) {
        return new StaticGeometryModel(owner_);
    });

    /**
     *
     */
    return true;
}

bool Graphics::useAsUIRenderScene(const ptr<scene::IRenderScene> scene_) {
    _uiRenderScene = scene_;
    return true;
}
