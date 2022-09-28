#include "Graphics.hpp"

#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Math/__default.inl>

#include "todo.h"
#include "Cache/GlobalCacheCtrl.hpp"
#include "Command/CommandBatch.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Loader/RevTextureLoader.hpp"
#include "Loader/StaticGeometryLoader.hpp"
#include "Loader/MaterialLoader.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/HORenderPass.hpp"
#include "Rev/GraphicPass/RevDepthPass.hpp"
#include "Rev/Renderer/RevRenderer.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"
#include "Texture/VkTextureFactory.hpp"
#include "Cache/GlobalResourceCache.hpp"

using namespace ember::engine::gfx;
using namespace ember::engine;
using namespace ember;

Graphics::Graphics(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _swapchain(nullptr),
    _computeQueue(nullptr),
    _graphicsQueue(nullptr),
    _transferQueue(nullptr),
    _cacheCtrl(nullptr),
    _renderer(nullptr),
    _camera(nullptr),
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
    auto* globalCache { cache::GlobalResourceCache::make(_device) };
    _cacheCtrl = make_uptr<cache::GlobalCacheCtrl>(globalCache);
    VkTextureFactory::make(_device);

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
     * Create Renderer
     */
    _renderer = new render::RevRenderer();
    _renderer->setup(_device);

    // Warning: Temporary
    _camera = new Camera();
    _camera->setPosition({ 8.F, /*-1.8F*/1.8F, 8.F });
    _camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
    _camera->setPerspective(60.F, static_cast<float>(_swapchain->width()) / static_cast<float>(_swapchain->height()),
        0.1F, 100.F);

    _camera->update();

    /**
     * Warning: Temporary
     */
    _renderPasses.clear();
    _renderPasses.reserve(_swapchain->length());

    for (u32 swap { 0 }; swap < _swapchain->length(); ++swap) {
        _renderPasses.push_back(_renderer->allocate({
            _swapchain->at(swap),
            nullptr,
            _camera
        }));
    }

    #if FALSE
    /**
     * Create Graphic Passes
     */
    _graphicPasses.resize(static_cast<u32>(GraphicPassMask::eFinalPass) + 1, nullptr);

    ptr<RevDepthPass> depthPass;

    _graphicPasses[static_cast<u8>(GraphicPassMask::eDepthPass)] = (depthPass = new RevDepthPass(_device, _swapchain));
    _graphicPasses[static_cast<u8>(GraphicPassMask::eLightPass)] = new RevLightPass(_device, _swapchain);
    _graphicPasses[static_cast<u8>(GraphicPassMask::eProbePass)] = new RevProbePass(_device, _swapchain);
    //_graphicPasses[static_cast<u8>(GraphicPassMask::eMainPass)] = new RevMainPass(_device, _swapchain, depthPass);
    _graphicPasses[static_cast<u8>(GraphicPassMask::eMainPass)] = new RevProbePass(_device, _swapchain);
    _graphicPasses[static_cast<u8>(GraphicPassMask::eFinalPass)] = new RevFinalPass(_device, _swapchain, depthPass);

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
            GraphicPassMask::eMainPass
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
    #endif

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

    // Warning: Temporary
    _renderPassBatches.clear();
    _renderPassBatches.reserve(_swapchain->length());

    for (u32 swap { 0 }; swap < _swapchain->length(); ++swap) {
        CommandBatch batch {};

        // batch.pushBarrier(_onFlightSync.image[swap]);
        batch.pushSignal(_onFlightSync.finish[swap]);
        batch.barrierStages() = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        _renderPassBatches.push_back(_STD move(batch));
    }

    /**
     * Signal Setup
     */
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
        // scheduler::thread::self::yield();
        scheduler::fiber::self::yield();
    }

    /**
     * Wait until graphics queues stopped execution
     */
    _device->computeQueue()->vkQueue().waitIdle();
    _device->graphicsQueue()->vkQueue().waitIdle();
    _device->transferQueue()->vkQueue().waitIdle();

    #if FALSE
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
    #endif

    // Warning: Temporary
    _renderPassBatches.clear();
    delete _camera;
    _camera = nullptr;

    /**
     * Render Passes
     */
    for (auto* entry : _renderPasses) {
        _renderer->free(_STD move(entry));
    }
    _renderPasses.clear();

    /**
     * Destroy device queues
     */
    _computeQueue->destroy();
    _graphicsQueue->destroy();
    _transferQueue->destroy();

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
     * Renderer
     */
    _renderer->destroy();
    delete _renderer;
    _renderer = nullptr;

    /**
     * Swapchain
     */
    delete _swapchain;
    _swapchain = nullptr;

    ShaderStorage::destroy();

    /**
     *
     */
    TextureFactory::destroy();
    _cacheCtrl.reset();
    cache::GlobalResourceCache::destroy();

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

const non_owning_rptr<gfx::cache::GlobalCacheCtrl> Graphics::cacheCtrl() const noexcept {
    return _cacheCtrl.get();
}

void Graphics::_tick(ptr<scene::IRenderScene> scene_) {

    SCOPED_STOPWATCH

    /**
     * 
     */
    static u32 syncIdx = 0;

    #if FALSE
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
    #endif

    // TODO: Make timeout as small as possible to skip blocking request -> if failed, return an reschedule tick task
    auto cpuGpuSync { _renderPasses[syncIdx]->unsafeSync() };
    if (cpuGpuSync) {
        #ifdef _DEBUG
        assert(
            _device->vkDevice().waitForFences(
                1, &cpuGpuSync,
                VK_TRUE, UINT64_MAX
            ) == vk::Result::eSuccess
        );
        //_device->vkDevice().resetFences(1, &cpuGpuSync);
        #else
        _device->vkDevice().waitForFences(1, &cpuGpuSync, VK_TRUE, UINT64_MAX);
        #endif
    }

    /**
     * 
     */
    u32 swapIdx { 0ui32 };
    [[maybe_unused]] auto result = _device->vkDevice().acquireNextImageKHR(
        static_cast<ptr<VkSwapchain>>(_swapchain)->vkSwapchain(),
        UINT64_MAX,
        _onFlightSync.image[syncIdx],
        nullptr,
        &swapIdx
    );

    _swapchain->setCurrentIdx(swapIdx);

    /**
     * Get Invocation / Await Last Render
     */
    auto* pass { _renderPasses[syncIdx] };

    #ifdef _DEBUG
    if (!pass->await()) {
        DEBUG_SNMSG(false, "WARN", "Failed to validate await state of render invocation.")
    }
    #endif

    /**
     * Warning: Temporary Update to change state
     */
    auto millis {
        _STD chrono::duration_cast<_STD chrono::milliseconds>(
            _STD chrono::high_resolution_clock::now().time_since_epoch()).count()
    };
    auto timeToVal { static_cast<double>(millis) / 5000.0 };

    _camera->setPosition({
        static_cast<float>(_STD sin(timeToVal)) * 8.F,
        /*-1.8F*/
        1.8F,
        static_cast<float>(_STD cos(timeToVal)) * 8.F
    });
    //_camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
    _camera->update();

    /**
     * Render
     */
    pass->use(_renderScene);
    pass->use(_camera);

    const auto* feedback = _renderer->invoke(pass, _renderPassBatches[syncIdx]);
    assert(feedback);

    #if FALSE
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
    #endif

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
    result = _graphicsQueue->vkQueue().presentKHR(present);
    assert(result == vk::Result::eSuccess);
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

#if FALSE
void Graphics::processGraphicPasses(ptr<scene::IRenderScene> scene_) {

    SCOPED_STOPWATCH

    constexpr GraphicPassMask masks[] {
        GraphicPassMask::eDepthPass,
        GraphicPassMask::eLightPass,
        GraphicPassMask::eProbePass,
        GraphicPassMask::eMainPass
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
            // TODO: get back dependencies for LORenderPass finish
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
#endif

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

#include "Scene/SceneTag.hpp"
#include "Scene/StaticGeometryModel.hpp"

bool Graphics::useAsRenderScene(const ptr<scene::IRenderScene> scene_) {
    _renderScene = scene_;

    if (scene_ == nullptr) {
        return true;
    }

    // TODO: Move callback register to other position and reconcider lifecycle \
    //  cause elements added to scene before callback register will be lost data

    /**
     *
     */
    auto* const scene { static_cast<const ptr<scene::RevScene>>(scene_) };
    scene->setNodeType(GfxSceneTag {}, StaticGeometryComponent::typeId,
        EmberObject::create<StaticGeometryModel, const ptr<SceneComponent>>);

    /**
     *
     */
    return true;
}

bool Graphics::useAsUIRenderScene(const ptr<scene::IRenderScene> scene_) {
    _uiRenderScene = scene_;
    return true;
}

void Graphics::registerLoader() {
    /**
     *
     */
    auto* manager { _session->modules().resourceManager() };
    auto& loader { manager->loader() };

    /**
     * Geometry Loader
     */
    const auto sgl = make_sptr<StaticGeometryLoader>(_cacheCtrl.get());

    loader.registerLoader<assets::StaticGeometry>(sgl);

    /**
     * Texture Loader
     */
    const auto rtl = make_sptr<RevTextureLoader>(_cacheCtrl.get());

    loader.registerLoader<assets::Texture>(rtl);

    /**
     * Material Loader
     */
    const auto mtl = make_sptr<MaterialLoader>(_cacheCtrl.get());

    loader.registerLoader<assets::GfxMaterial>(mtl);
}

void Graphics::registerImporter() {}
