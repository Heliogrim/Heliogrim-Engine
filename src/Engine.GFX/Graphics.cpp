#include "Graphics.hpp"

#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Math/__default.inl>
#include <Engine.GFX.Glow.3D/Renderer/RevRenderer.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>

#include "todo.h"
#include "Cache/GlobalCacheCtrl.hpp"
#include "Cache/GlobalResourceCache.hpp"
#include "Command/CommandBatch.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Loader/MaterialLoader.hpp"
#include "Loader/RevTextureLoader.hpp"
#include "Loader/StaticGeometryLoader.hpp"
#include "Renderer/HORenderPass.hpp"
#include "Renderer/Renderer.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Swapchain/VkSwapchain.hpp"
#include "Texture/VkTextureFactory.hpp"
#include "RenderTarget.hpp"

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
    _uiRenderer = new glow::ui::render::UiRenderer();
    _uiRenderer->setup(_device);

    _renderer = new glow::render::RevRenderer();
    _renderer->setup(_device);

    // Warning: Temporary
    _camera = new Camera();
    _camera->setPosition({ 8.F, /*-1.8F*/1.8F, 8.F });
    _camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
    _camera->setPerspective(60.F, static_cast<float>(_swapchain->width()) / static_cast<float>(_swapchain->height()),
        0.1F, 100.F);

    _camera->update();

    /**
     * Setup (Main) RenderTarget
     */
    _renderTarget = make_ptr<RenderTarget>();

    _renderTarget->use(_device);
    _renderTarget->use(&_surface);
    _renderTarget->use(_swapchain);
    _renderTarget->use(_renderer);

    _renderTarget->rebuildPasses(_camera);

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

    /**
     * Cleanup (Main) RenderTarget
     */
    delete _renderTarget;
    _renderTarget = nullptr;

    // Warning: Temporary
    delete _camera;
    _camera = nullptr;

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

void Graphics::_tick(ptr<scene::IRenderScene> scene_) {

    SCOPED_STOPWATCH

    /**
     * Tick (Main) RenderTarget
     */
    auto* renderPass = _renderTarget->next();

    /**
     * Warning: Temporary Update to change state
     */
    {
        auto millis {
            _STD chrono::duration_cast<_STD chrono::milliseconds>(
                _STD chrono::high_resolution_clock::now().time_since_epoch()).count()
        };
        auto timeToVal { static_cast<double>(millis) / 5000.0 };

        constexpr auto dist { 3.F };
        _camera->setPosition({
            static_cast<float>(_STD sin(timeToVal)) * dist,
            /*-1.8F*/
            /*1.2F*/
            0.F,
            static_cast<float>(_STD cos(timeToVal)) * dist
        });
        //_camera->setLookAt({ 0.F, /*-1.8F*/0.F, 0.F });
        _camera->update();
    }

    /**
     *
     */
    renderPass->use(scene_);
    renderPass->use(_camera);

    _renderTarget->update();
    _renderTarget->finish({});

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

#include <Ember/SkyboxComponent.hpp>
#include <Ember/StaticGeometryComponent.hpp>
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
