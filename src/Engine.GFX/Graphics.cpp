#include "Graphics.hpp"

#include <Engine.Scheduler/Async.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldChangeEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>
#include <Engine.GFX.Glow.3D/Renderer/RevRenderer.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>
#include <Engine.GFX.Loader/Manager.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX.Schedule/RenderScenePipeline.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.GFX.Renderer/HORenderPass.hpp>
#include <Engine.GFX.Renderer/Renderer_Deprecated.hpp>

#include "RenderTarget.hpp"
#include "todo.h"
#include "Cache/GlobalCacheCtrl.hpp"
#include "Cache/GlobalResourceCache.hpp"
#include "Command/CommandBatch.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/World.hpp"
#include "Engine.Event/GlobalEventEmitter.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Scene/IRenderScene.hpp"
#include "Engine.Scene/RevScene.hpp"
#include "Engine.Scene/Scene.hpp"
#include "Importer/ImageFileTypes.hpp"
#include "Importer/ImageImporter.hpp"
#include "Importer/ModelFileTypes.hpp"
#include "Importer/ModelImporter.hpp"
#include "Pool/GlobalResourcePool.hpp"
#include "Shader/ShaderStorage.hpp"
#include "Surface/SurfaceManager.hpp"
#include "Swapchain/Swapchain.hpp"
#include "Texture/VkTextureFactory.hpp"

#include "Editor.GFX/Renderer/EdRevRenderer.hpp"

/* Development Testing */
#include <Engine.GFX.RenderGraph/test.hpp>
#include <Engine.GFX.RenderPipeline/test.hpp>
/**/

using namespace hg::engine::gfx;
using namespace hg::engine;
using namespace hg;

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

    const auto owae = _engine->getEmitter().on<core::WorldAddedEvent>(
        [this](cref<core::WorldAddedEvent> event_) {

            auto* const scene { event_.getWorld()->getScene() };
            const auto* const sceneClass { scene->getMetaClass() };

            /*
            if (not sceneClass->isType<scene::IRenderScene>()) {
                return;
            }
             */

            if (not sceneClass->exact<scene::RevScene>()) {
                return;
            }

            _sceneManager->registerScene(static_cast<const ptr<scene::RevScene>>(scene));
        }
    );
    _hooks.push_back({ core::WorldAddedEvent::typeId.data, owae });

    const auto owre = _engine->getEmitter().on<core::WorldRemoveEvent>(
        [this](cref<core::WorldRemoveEvent> event_) {

            auto* const scene { event_.getWorld()->getScene() };
            const auto* const sceneClass { scene->getMetaClass() };

            /*
            if (not sceneClass->isType<scene::IRenderScene>()) {
                return;
            }
             */

            if (not sceneClass->exact<scene::RevScene>()) {
                return;
            }

            _sceneManager->unregisterScene(static_cast<const ptr<scene::RevScene>>(scene));
            cleanupTargetsByScene(static_cast<const ptr<scene::RevScene>>(scene));
        }
    );
    _hooks.push_back({ core::WorldRemoveEvent::typeId.data, owre });

    const auto owce = _engine->getEmitter().on<core::WorldChangeEvent>(
        [this](cref<core::WorldChangeEvent> event_) {

            event_.getSession();
            event_.getPrevWorld();
            event_.getNextWorld();

        }
    );
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

    test_render_graph();
    test_render_pipe();

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
    auto globalCache = make_uptr<cache::GlobalResourceCache>(_device);
    _cacheCtrl = make_uptr<cache::GlobalCacheCtrl>(_STD move(globalCache));

    _pool = make_uptr<pool::GlobalResourcePool>(_device);

    VkTextureFactory::make(_device);

    /**
     * Create Shader Storage
     */
    ShaderStorage::make();
    build_shader(_device);

    /**
     * Render Scenes
     */
    _sceneManager = ::gfx::scene::RenderSceneManager::make();

    /**
     * Register Hooks and consume EngineState
     */
    hookEngineState();

    auto& loader = _engine->getResources()->loader();
    loader::register_loader(loader, this, _cacheCtrl.get(), _pool.get());
    registerImporter();

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
        //auto renderer = make_sptr<hg::editor::gfx::EdRevRenderer>();
        renderer->setup(_device);

        _cachedRenderer.insert_or_assign(AssocKey<string>::from("3DRenderer"), _STD move(renderer));
    }

    /**
     * Scheduling Pipelines
     */
    auto renderScenePipeline = make_uptr<gfx::schedule::RenderScenePipeline>();
    _engine->getScheduler()->getCompositePipeline()->addPipeline(_STD move(renderScenePipeline));
}

void Graphics::start() {}

void Graphics::stop() {}

void Graphics::destroy() {
    SCOPED_STOPWATCH

    /**
     * Unregister Hooks and consume EngineState
     */
    unregisterImporter();

    auto& loader = _engine->getResources()->loader();
    loader::unregister_loader(loader, this, _cacheCtrl.get(), _pool.get());
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
    _pool.reset();

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

sptr<gfx::render::Renderer_Deprecated> Graphics::getRenderer(cref<AssocKey<string>> key_) const {
    return _cachedRenderer.at(key_);
}

sptr<gfx::render::Renderer_Deprecated> Graphics::getRenderer(cref<AssocKey<string>> key_, std::nothrow_t) const noexcept {
    const auto it { _cachedRenderer.find(key_) };
    return it != _cachedRenderer.end() ? it->second : nullptr;
}

bool Graphics::hasRenderer(cref<AssocKey<string>> key_) {
    return _cachedRenderer.contains(key_);
}

bool Graphics::removeRenderer(cref<AssocKey<string>> key_) {
    return _cachedRenderer.erase(key_);
}

const non_owning_rptr<gfx::scene::RenderSceneManager> Graphics::getSceneManager() const noexcept {
    return _sceneManager;
}

void Graphics::cleanupTargetsByScene(const non_owning_rptr<scene::IRenderScene> scene_) {

    CompactArray<sptr<RenderTarget>> targets {};
    _sceneManager->selectPrimaryTargets(
        targets,
        [scene_](cref<sptr<RenderTarget>> target_) {
            // TODO: Check whether we want this with a query to the render targets to check whether
            // TODO:    the render target is effected by scene changes ( deletion )
            // return target_->hasScene(scene_);
            return true;
        }
    );

    for (auto&& target : targets) {
        target->setActive(false);
        _sceneManager->dropPrimaryTarget(target);
    }

    /**/

    targets.clear();
    _sceneManager->selectSecondaryTargets(
        targets,
        [scene_](cref<sptr<RenderTarget>> target_) {
            // TODO: Check whether we want this with a query to the render targets to check whether
            // TODO:    the render target is effected by scene changes ( deletion )
            // return target_->hasScene(scene_);
            return true;
        }
    );

    for (auto&& target : targets) {
        target->setActive(false);
        _sceneManager->dropSecondaryTarget(target);
    }
}

const non_owning_rptr<gfx::SurfaceManager> Graphics::getSurfaceManager() const noexcept {
    return _surfaceManager.get();
}

void Graphics::registerImporter() {
    /**/
    auto* manager { _engine->getResources() };
    auto& importer { manager->importer() };

    importer.registerImporter(gfx::ImageFileType::Ktx2, make_sptr<KtxImporter>());
    importer.registerImporter(gfx::ModelFileType::Fbx, make_sptr<FbxImporter>());
}

void Graphics::unregisterImporter() {

    auto* const manager { _engine->getResources() };
    auto& importer { manager->importer() };

    importer.unregisterImporter(gfx::ImageFileType::Ktx2);
    importer.unregisterImporter(gfx::ModelFileType::Fbx);
}
