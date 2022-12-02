#include "Graphics.hpp"

#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.GFX.Glow.3D/Renderer/RevRenderer.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>
#include <Engine.Logging/Logger.hpp>

#include "RenderTarget.hpp"
#include "todo.h"
#include "Cache/GlobalCacheCtrl.hpp"
#include "Cache/GlobalResourceCache.hpp"
#include "Command/CommandBatch.hpp"
#include "Engine.Resource/ResourceManager.hpp"
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
#include "Swapchain/VkSurfaceSwapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"
#include "Texture/VkTextureFactory.hpp"

#if TRUE
#include <Editor.GFX/Renderer/EdRevRenderer.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember::engine;
using namespace ember;

Graphics::Graphics(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _swapchain(nullptr),
    _secondarySwapchain(nullptr),
    _computeQueue(nullptr),
    _graphicsQueue(nullptr),
    _transferQueue(nullptr),
    _cacheCtrl(nullptr),
    _renderer(nullptr),
    _uiRenderer(nullptr),
    _camera(nullptr),
    _renderTarget(nullptr),
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
    _swapchain = new VkSurfaceSwapchain();
    static_cast<ptr<VkSurfaceSwapchain>>(_swapchain)->useSurface(_surface);
    _swapchain->setup(_device);

    /**
     * Create Shader Storage
     */
    ShaderStorage::make();
    build_shader(_device);

    /**
     * Create Renderer
     */
    _uiRenderer = new glow::ui::render::UiRenderer();
    _uiRenderer->setup(_device);

    //_renderer = new glow::render::RevRenderer();
    _renderer = new editor::gfx::EdRevRenderer();
    _renderer->setup(_device);

    // Warning: Temporary
    _camera = make_sptr<Camera>();
    _camera->setPosition({ 0.F, /*-1.8F*/2.5F, -5.F });
    _camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
    _camera->setPerspective(60.F,
        static_cast<float>(_swapchain->extent().x) / static_cast<float>(_swapchain->extent().y),
        0.01F, 100.F);

    _camera->update();

    /**
     * Setup (Main) RenderTarget
     */
    _renderTarget = make_sptr<RenderTarget>();

    _renderTarget->use(_device);
    _renderTarget->use(&_surface);
    //_renderTarget->use(_swapchain);
    _renderTarget->use(_renderer);

    //_renderTarget->buildPasses(_camera);

    #if TRUE
    /**
     * Setup (Main) UI RenderTarget
     */
    _uiRenderTarget = make_sptr<RenderTarget>();

    _uiRenderTarget->use(_device);
    _uiRenderTarget->use(&_surface);
    _uiRenderTarget->use(_swapchain);
    _uiRenderTarget->use(_uiRenderer);

    _uiRenderTarget->buildPasses(_camera.get());
    #endif

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

    #if TRUE
    /**
     * Cleanup (Main) UI RenderTarget
     */
    _uiRenderTarget.reset();
    #endif

    /**
     * Cleanup (Main) RenderTarget
     */
    _renderTarget.reset();

    // Warning: Temporary
    _camera.reset();

    /**
     * Destroy device queues
     */
    _computeQueue->destroy();
    _graphicsQueue->destroy();
    _transferQueue->destroy();

    /**
     * Renderer
     */
    _renderer->destroy();
    delete _renderer;
    _renderer = nullptr;

    _uiRenderer->destroy();
    delete _uiRenderer;
    _uiRenderer = nullptr;

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

void Graphics::tick(
    cref<sptr<gfx::RenderTarget>> target_,
    ptr<scene::IRenderScene> scene_,
    ptr<gfx::Camera> camera_
) const {

    SCOPED_STOPWATCH

    /**
     * Tick RenderTarget
     */
    if (not target_->ready()) {
        IM_DEBUG_LOG(
            "Tried to tick an unready RenderTarget. Please ensure the completness of RenderTargets before ticking/dispatching...");
        return;
    }

    auto* renderPass = target_->next();

    if (not renderPass) {
        IM_DEBUG_LOG("Skipping graphics tick due to missing RenderPass (No next Swapchain Image) at RenderTarget");
        return;
    }

    renderPass->use(scene_);
    renderPass->use(camera_);

    target_->update();

    const auto result = target_->finish({});
    assert(result != RenderEnqueueResult::eFailed);
}

#if TRUE
// Warning: Temporary Experimental
void reportStats(float fps_, float time_);
#endif

void Graphics::_tick() {

    #if FALSE
    /**
     * Warning: Temporary Update to change state
     */
    {
        auto millis {
            _STD chrono::duration_cast<_STD chrono::milliseconds>(
                _STD chrono::high_resolution_clock::now().time_since_epoch()).count()
        };
        auto timeToVal { static_cast<double>(millis) / 5000.0 };

        constexpr auto dist { 5.F };
        _camera->setPosition({
            static_cast<float>(_STD sin(timeToVal)) * dist,
            /*-1.8F*/
            0.6F,
            static_cast<float>(_STD cos(timeToVal)) * dist
        });
        //_camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
        _camera->update();
    }
    #endif

    for (const auto& entry : _scheduledTargets) {
        tick(entry.target, entry.scene, entry.camera);
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

void Graphics::__tmp__resize(cref<math::uivec2> extent_) {

    /**
     * Create a new swapchain to present image
     */
    auto* nextSwapchain = new VkSurfaceSwapchain();
    nextSwapchain->useSurface(_surface);
    nextSwapchain->setup(_device);

    /**
     * Block Render Passes related to swapchain to rebuild passes
     */
    auto succeeded = _uiRenderTarget->rebuildPasses(nextSwapchain);
    assert(succeeded);

    /**
     * Cleanup
     */
    _swapchain->destroy();
    delete _swapchain;

    _swapchain = nextSwapchain;
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
                    _tick();
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

#include <Ember/SkyboxComponent.hpp>
#include <Ember/StaticGeometryComponent.hpp>
#include <Ember/UIComponent.hpp>
#include <Engine.GFX.Glow.UI/Scene/UISceneModel.hpp>
#include <Engine.Scene/RevScene.hpp>

#include "Scene/SceneTag.hpp"
#include "Scene/SkyboxModel.hpp"
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

    scene->setNodeType(GfxSceneTag {}, SkyboxComponent::typeId,
        EmberObject::create<SkyboxModel, const ptr<SceneComponent>>);

    /**/

    const auto stit {
        _STD find_if(_scheduledTargets.begin(), _scheduledTargets.end(),
            [renderTarget = _renderTarget](const auto& entry) {
                return entry.target == renderTarget;
            })
    };
    if (stit != _scheduledTargets.end()) {
        _scheduledTargets.erase(stit);
    }

    _scheduledTargets.push_back({ _renderTarget, scene_, _camera.get() });

    /**/
    return true;
}

bool Graphics::useAsUIRenderScene(const ptr<scene::IRenderScene> scene_) {
    _uiRenderScene = scene_;

    if (scene_ == nullptr) {
        return true;
    }

    // TODO: Move callback register to other position and reconcider lifecycle \
    //  cause elements added to scene before callback register will be lost data

    /**
     *
     */
    auto* const scene { static_cast<const ptr<scene::RevScene>>(scene_) };

    scene->setNodeType(GfxSceneTag {}, UIComponent::typeId,
        EmberObject::create<glow::ui::UISceneModel, const ptr<SceneComponent>>);

    /**/

    const auto stit {
        _STD find_if(_scheduledTargets.begin(), _scheduledTargets.end(),
            [renderTarget = _uiRenderTarget](const auto& entry) {
                return entry.target == renderTarget;
            })
    };
    if (stit != _scheduledTargets.end()) {
        _scheduledTargets.erase(stit);
    }

    _scheduledTargets.push_back({ _uiRenderTarget, scene_, _camera.get() });

    /**/
    return true;
}

void Graphics::registerLoader() {
    /**
     *
     */
    auto* manager { _session->modules().resourceManager() };
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

    /**
     *
     */
    auto* manager { _session->modules().resourceManager() };
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
