#include "Graphics.hpp"

#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.GFX.Glow.3D/Renderer/RevRenderer.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>
#include <Engine.Core/Event/WorldChangeEvent.hpp>

#include "RenderTarget.hpp"
#include "todo.h"
#include "Cache/GlobalCacheCtrl.hpp"
#include "Cache/GlobalResourceCache.hpp"
#include "Command/CommandBatch.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/World.hpp"
#include "Engine.Event/GlobalEventEmitter.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Scene/Scene.hpp"
#include "Loader/FontLoader.hpp"
#include "Loader/MaterialLoader.hpp"
#include "Loader/RevTextureLoader.hpp"
#include "Loader/StaticGeometryLoader.hpp"
#include "Importer/ImageImporter.hpp"
#include "Importer/ImageFileTypes.hpp"
#include "Renderer/HORenderPass.hpp"
#include "Renderer/Renderer.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Texture/VkTextureFactory.hpp"
#include "Surface/SurfaceManager.hpp"
#include "Engine.Scene/IRenderScene.hpp"
#include "Engine.Scene/RevScene.hpp"

using namespace ember::engine::gfx;
using namespace ember::engine;
using namespace ember;

Graphics::Graphics(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_),
    _computeQueue(nullptr),
    _graphicsQueue(nullptr),
    _transferQueue(nullptr),
    _cacheCtrl(nullptr),
    _sceneManager(nullptr),
    _surfaceManager(nullptr) {}

Graphics::~Graphics() {
    tidy();
}

void Graphics::tidy() {
    __noop();
}

void Graphics::hookEngineState() {

    const auto owae = _engine->getEmitter().on<core::WorldAddedEvent>([this](cref<core::WorldAddedEvent> event_) {

        auto* const scene { event_.getWorld()->getScene() };
        const auto* const sceneClass { scene->getClass() };

        /*
        if (not sceneClass->isType<scene::IRenderScene>()) {
            return;
        }
         */

        if (not sceneClass->isExactType<scene::RevScene>()) {
            return;
        }

        _sceneManager->registerScene(static_cast<const ptr<scene::RevScene>>(scene));
    });
    _hooks.push_back({ core::WorldAddedEvent::typeId.data, owae });

    const auto owre = _engine->getEmitter().on<core::WorldRemoveEvent>([this](cref<core::WorldRemoveEvent> event_) {

        auto* const scene { event_.getWorld()->getScene() };
        const auto* const sceneClass { scene->getClass() };

        /*
        if (not sceneClass->isType<scene::IRenderScene>()) {
            return;
        }
         */

        if (not sceneClass->isExactType<scene::RevScene>()) {
            return;
        }

        _sceneManager->unregisterScene(static_cast<const ptr<scene::RevScene>>(scene));
    });
    _hooks.push_back({ core::WorldRemoveEvent::typeId.data, owre });

    const auto owce = _engine->getEmitter().on<core::WorldChangeEvent>([this](cref<core::WorldChangeEvent> event_) {

        event_.getSession();
        event_.getPrevWorld();
        event_.getNextWorld();

    });
    _hooks.push_back({ core::WorldChangeEvent::typeId.data, owce });
}

void Graphics::unhookEngineState() {

    for (const auto& entry : _hooks) {
        switch (entry.first) {
            case core::WorldAddedEvent::typeId.data: {
                _engine->getEmitter().remove<core::WorldAddedEvent>(entry.second);
                break;
            }
            case core::WorldRemoveEvent::typeId.data: {
                _engine->getEmitter().remove<core::WorldChangeEvent>(entry.second);
                break;
            }
            case core::WorldChangeEvent::typeId.data: {
                _engine->getEmitter().remove<core::WorldChangeEvent>(entry.second);
                break;
            }
        }
    }

    _hooks.clear();
}

void Graphics::setup() {
    SCOPED_STOPWATCH

    /**
     * Create a new application
     */
    _application = {};
    _application.setup();

    /**
     * Surfaces
     */
    _surfaceManager = make_uptr<SurfaceManager>(&_application);

    /**
     * Prepare rendering specific data structures
     *
     *	a: create a new device to access graphics architecture
     */
    _device = make_sptr<Device>(_application);
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
     * Create Shader Storage
     */
    ShaderStorage::make();
    build_shader(_device);

    /**
     * Create Renderer
     */
    {
        auto renderer = make_sptr<glow::ui::render::UiRenderer>();
        renderer->setup(_device);

        _cachedRenderer.insert_or_assign(AssocKey<string>::from("UIRenderer"), _STD move(renderer));
    }

    {
        auto renderer = make_sptr<glow::render::RevRenderer>();
        //auto renderer = make_sptr<editor::gfx::EdRevRenderer>();
        renderer->setup(_device);

        _cachedRenderer.insert_or_assign(AssocKey<string>::from("3DRenderer"), _STD move(renderer));
    }

    /**
     * Render Scenes
     */
    _sceneManager = ::gfx::scene::RenderSceneManager::make();

    /**
     * Register Hooks and consume EngineState
     */
    hookEngineState();
    registerLoader();
    registerImporter();

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

void Graphics::desync() {}

void Graphics::destroy() {
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
     * Unregister Hooks and consume EngineState
     */
    unhookEngineState();

    /**
     * Surfaces
     */
    _surfaceManager.reset();

    /**
     * Wait until graphics queues stopped execution
     */
    _device->computeQueue()->vkQueue().waitIdle();
    _device->graphicsQueue()->vkQueue().waitIdle();
    _device->transferQueue()->vkQueue().waitIdle();

    /**
     * Destroy device queues
     */
    _computeQueue->destroy();
    _graphicsQueue->destroy();
    _transferQueue->destroy();

    /**
     * Renderer
     */
    for (const auto& entry : _cachedRenderer) {
        entry.second->destroy();
    }
    _cachedRenderer.clear();

    /**/
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

    _application.destroy();
}

sptr<Device> Graphics::getCurrentDevice() const noexcept {
    return _device;
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

sptr<gfx::render::Renderer> Graphics::getRenderer(cref<AssocKey<string>> key_) const {
    return _cachedRenderer.at(key_);
}

sptr<gfx::render::Renderer> Graphics::getRenderer(cref<AssocKey<string>> key_, std::nothrow_t) const noexcept {
    const auto it { _cachedRenderer.find(key_) };
    return it != _cachedRenderer.end() ? it->second : nullptr;
}

bool Graphics::hasRenderer(cref<AssocKey<string>> key_) {
    return _cachedRenderer.contains(key_);
}

bool Graphics::removeRenderer(cref<AssocKey<string>> key_) {
    return _cachedRenderer.erase(key_);
}

#if TRUE
// Warning: Temporary Experimental
void reportStats(float fps_, float time_);
#endif

const non_owning_rptr<gfx::scene::RenderSceneManager> Graphics::getSceneManager() const noexcept {
    return _sceneManager;
}

const non_owning_rptr<gfx::SurfaceManager> Graphics::getSurfaceManager() const noexcept {
    return _surfaceManager.get();
}

void Graphics::tick() {
    CompactSet<sptr<RenderTarget>> targets {};
    _sceneManager->selectInvokeTargets(targets);

    for (const auto& target : targets) {
        invokeRenderTarget(target);
    }

    /**
     * Debug Metrics
     */
    static concurrent::RingBuffer<_STD chrono::nanoseconds> timeBuffer { 17 };

    static auto lastFrame = std::chrono::high_resolution_clock::now();
    const auto now = _STD chrono::high_resolution_clock::now();

    auto duration { _STD chrono::duration_cast<_STD chrono::nanoseconds>(now - lastFrame) };
    timeBuffer.try_push(_STD move(duration));

    if (timeBuffer.full()) {
        _STD chrono::nanoseconds tmp {};
        timeBuffer.try_pop(tmp);
    }

    lastFrame = now;

    /**/

    _STD chrono::nanoseconds accValue {};
    u32 accCount { 0ui32 };
    for (const auto& entry : timeBuffer.unsafe_container()) {
        if (entry.count() > 0) {
            accValue += entry;
            ++accCount;
        }
    }

    const double avg { static_cast<double>(accValue.count()) / static_cast<double>(accCount) };
    const double diff { static_cast<double>(duration.count()) / 1000'000.0 };

    reportStats(static_cast<float>(1000'000'000.0 / avg), static_cast<float>(diff));
}

void Graphics::invokeRenderTarget(cref<sptr<gfx::RenderTarget>> target_) const {
    SCOPED_STOPWATCH

    /**
     * Tick RenderTarget
     */
    if (not target_->ready()) {
        IM_DEBUG_LOG(
            "Tried to tick an unready RenderTarget. Please ensure the completness of RenderTargets before ticking/dispatching...");
        return;
    }

    const auto* renderPass = target_->next();

    if (not renderPass) {
        IM_DEBUG_LOG("Skipping graphics tick due to missing RenderPass (No next Swapchain Image) at RenderTarget");
        return;
    }

    target_->update();

    const auto result = target_->finish({});
    assert(result != RenderEnqueueResult::eFailed);
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
                    tick();
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

void Graphics::registerLoader() {
    /**/
    auto* manager { _engine->getResources() };
    auto& loader { manager->loader() };

    /**
     * Font Loader
     */
    const auto fl = make_sptr<FontLoader>(_cacheCtrl.get());

    loader.registerLoader<assets::Font>(fl);

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

void Graphics::registerImporter() {
    /**/
    auto* manager { _engine->getResources() };
    auto& importer { manager->importer() };

    importer.registerImporter(gfx::ImageFileType::Ktx2, make_ptr<KtxImporter>());
}

#if TRUE
// Warning: Experimental Test

#include <format>
#include <Engine.GFX.Glow.UI/TestUI.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

void reportStats(float fps_, float time_) {
    if (not testFrameDisplay.expired()) {
        auto* tfd { static_cast<ptr<engine::reflow::Text>>(testFrameDisplay.lock().get()) };
        tfd->setText(std::format("{:.0f} FPS", fps_));
    }

    if (not testFrameTime.expired()) {
        auto* tft { static_cast<ptr<engine::reflow::Text>>(testFrameTime.lock().get()) };
        tft->setText(std::format("{:.2f} ms", time_));
    }
}
#endif
