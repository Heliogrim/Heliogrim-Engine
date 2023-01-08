#include "EditorEngine.hpp"

#include <Editor.Main/Boot/SubModuleInit.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Core/EngineState.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Helper/Wait.hpp>
#include <Engine.SFX/Audio.hpp>

#include "Engine.Core/World.hpp"
#include "Engine.Core/WorldContext.hpp"
#include "Engine.Scene/Scene.hpp"

#ifdef WIN32
#include <Engine.Platform/Windows/WinPlatform.hpp>
#else
#include <Engine.Platform/Linux>
#endif

using namespace ember::editor;
using namespace ember::engine::core;
using namespace ember::engine;
using namespace ember;

EditorEngine::EditorEngine() = default;

EditorEngine::~EditorEngine() = default;

non_owning_rptr<EditorEngine> EditorEngine::getEngine() noexcept {
    return static_cast<ptr<EditorEngine>>(_cached);
}

bool EditorEngine::preInit() {
    if (getEngineState() > EngineState::eUndefined) {
        return false;
    }

    #ifdef _DEBUG
    if (_assets || _audio || _graphics || _input || _network || _physics || _resources || _scheduler) {
        return false;
    }
    #endif

    #ifdef WIN32
    _platform = make_uptr<WinPlatform>();
    #else
    _platform = nullptr;
    #endif

    _resources = make_uptr<ResourceManager>();
    _scheduler = make_uptr<CompScheduler>();

    /**/

    _assets = make_uptr<Assets>(this);
    _audio = make_uptr<Audio>(this);
    _graphics = make_uptr<Graphics>(this);
    _input = make_uptr<Input>(this);
    _network = make_uptr<Network>(this);
    _physics = make_uptr<Physics>(this);

    /**/

    boot::preInitSubModules();

    const auto result = _modules.validate();
    if (result == DependencyValidationResult::eSuccess) {
        IM_CORE_LOG("Module dependencies and order resolved correctly.");
    } else if (result == DependencyValidationResult::eFailedRequired) {
        IM_CORE_ERROR("Failed to resolve all module dependencies.");
    } else {
        IM_CORE_ERROR("Module dependency and order resolving failed.");
    }

    /**/

    return setEngineState(EngineState::ePreInitialized);
}

bool EditorEngine::init() {
    if (getEngineState() != EngineState::ePreInitialized) {
        return false;
    }

    /* Base module are setup via direct call without fiber context guarantee (which is unlikely) */
    _platform->setup();
    _scheduler->setup(Scheduler::auto_worker_count);
    _resources->setup();

    /**/

    setupCorePipelines();

    /* Core modules should always interact with a guaranteed fiber context and non-sequential execution */
    _STD atomic_uint_fast8_t setupCounter { 0ui8 };

    scheduler::exec([this, &setupCounter] {
        _assets->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    scheduler::exec([this, &setupCounter] {
        _audio->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    scheduler::exec([this, &setupCounter] {
        _graphics->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    scheduler::exec([this, &setupCounter] {
        _input->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    scheduler::exec([this, &setupCounter] {
        _network->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    scheduler::exec([this, &setupCounter] {
        _physics->setup();
        ++setupCounter;
        setupCounter.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(setupCounter, 6ui8);
    setupCounter.store(0ui8, _STD memory_order_relaxed);

    scheduler::exec([this, &setupCounter] {

        for (const auto& subModule : _modules.getSubModules()) {
            subModule->setup();
        }

        ++setupCounter;
        setupCounter.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(setupCounter, 1ui8);
    return setEngineState(EngineState::eInitialized);
}

bool EditorEngine::postInit() {
    if (getEngineState() != EngineState::eInitialized) {
        return false;
    }

    /**/
    _scheduler->finalize();

    // TODO: Asset seeder resolving
    // TODO: 

    return setEngineState(EngineState::ePostInitialized);
}

bool EditorEngine::start() {
    if (getEngineState() != EngineState::ePostInitialized) {
        return false;
    }

    /**/
    _STD atomic_flag next {};
    scheduler::exec([this, &next] {
        _scheduler->getCompositePipeline()->start();

        next.test_and_set(_STD memory_order_relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    next.clear(std::memory_order::release);

    /**/
    scheduler::exec([this, &next] {
        _assets->start();
        _audio->start();
        _graphics->start();
        _input->start();
        _network->start();
        _physics->start();

        next.test_and_set(std::memory_order::relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    next.clear(std::memory_order::release);

    /**/
    scheduler::exec([this, &next] {
        _editorSession = make_uptr<core::Session>();
        _worldContexts.push_back(_editorSession->getWorldContext());

        _primaryGameSession = make_uptr<core::Session>();
        _worldContexts.push_back(_primaryGameSession->getWorldContext());

        /**/

        for (const auto& subModule : _modules.getSubModules()) {
            subModule->start();
        }

        next.test_and_set(std::memory_order::relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    return setEngineState(EngineState::eStarted);
}

bool EditorEngine::stop() {
    if (getEngineState() != EngineState::eStarted) {
        return false;
    }

    /**/
    _STD atomic_flag next {};
    scheduler::exec([this, &next] {
        _physics->stop();
        _network->stop();
        _input->stop();
        _graphics->stop();
        _audio->stop();
        _assets->stop();

        next.test_and_set(std::memory_order::relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    next.clear(std::memory_order::release);

    /**/
    scheduler::exec([this, &next] {

        const auto& modules = _modules.getSubModules();
        for (auto iter = modules.rbegin(); iter != modules.rend(); ++iter) {
            (*iter)->stop();
        }

        /**/

        auto prevWorld = _primaryGameSession->getWorldContext()->getCurrentWorld();
        _primaryGameSession->getWorldContext()->setCurrentWorld(nullptr);
        removeWorld(prevWorld);

        auto where = std::ranges::remove(_worldContexts, _primaryGameSession->getWorldContext());
        _worldContexts.erase(where.begin(), where.end());
        _primaryGameSession.reset();

        /**/

        prevWorld = _editorSession->getWorldContext()->getCurrentWorld();
        _editorSession->getWorldContext()->setCurrentWorld(nullptr);
        removeWorld(prevWorld);

        where = std::ranges::remove(_worldContexts, _editorSession->getWorldContext());
        _worldContexts.erase(where.begin(), where.end());
        _editorSession.reset();

        next.test_and_set(std::memory_order::relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    next.clear(std::memory_order::release);

    /**/
    scheduler::exec([this, &next] {
        _scheduler->getCompositePipeline()->stop();

        next.test_and_set(_STD memory_order_relaxed);
        next.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(next, true);
    return setEngineState(EngineState::eStopped);
}

bool EditorEngine::shutdown() {
    if (getEngineState() != EngineState::eStopped && getEngineState() != EngineState::eInitialized) {
        return false;
    }

    _STD atomic_flag subModuleFlag {};
    scheduler::exec([this, &subModuleFlag] {

        const auto& modules = _modules.getSubModules();
        for (auto iter = modules.rbegin(); iter != modules.rend(); ++iter) {
            (*iter)->destroy();
        }

        subModuleFlag.test_and_set(_STD memory_order_relaxed);
        subModuleFlag.notify_one();
    });

    scheduler::waitOnAtomic(subModuleFlag, false);

    /* Core modules should always interact with a guaranteed fiber context and non-sequential execution */
    _STD atomic_uint_fast8_t moduleCount { 0ui8 };

    scheduler::exec([this, &moduleCount] {
        _physics->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    scheduler::exec([this, &moduleCount] {
        _network->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    scheduler::exec([this, &moduleCount] {
        _input->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    scheduler::exec([this, &moduleCount] {
        _graphics->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    scheduler::exec([this, &moduleCount] {
        _audio->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    scheduler::exec([this, &moduleCount] {
        _assets->destroy();
        ++moduleCount;
        moduleCount.notify_one();
    });

    /**/
    scheduler::waitUntilAtomic(moduleCount, 6ui8);

    /* Base module are setup via direct call without fiber context guarantee (which is unlikely) */
    _resources->destroy();
    _scheduler->destroy();
    _platform->tidy();

    return setEngineState(EngineState::eShutdown);
}

bool EditorEngine::exit() {
    if (getEngineState() != EngineState::eShutdown) {
        return false;
    }

    /**/
    _physics.reset();
    _network.reset();
    _input.reset();
    _graphics.reset();
    _audio.reset();
    _assets.reset();

    /**/
    _platform.reset();
    _scheduler.reset();
    _resources.reset();

    return setEngineState(EngineState::eExited);
}

non_owning_rptr<engine::Assets> EditorEngine::getAssets() const noexcept {
    return _assets.get();
}

non_owning_rptr<engine::Audio> EditorEngine::getAudio() const noexcept {
    return _audio.get();
}

non_owning_rptr<engine::Graphics> EditorEngine::getGraphics() const noexcept {
    return _graphics.get();
}

non_owning_rptr<engine::Input> EditorEngine::getInput() const noexcept {
    return _input.get();
}

non_owning_rptr<engine::Network> EditorEngine::getNetwork() const noexcept {
    return _network.get();
}

non_owning_rptr<engine::Physics> EditorEngine::getPhysics() const noexcept {
    return _physics.get();
}

non_owning_rptr<engine::Platform> EditorEngine::getPlatform() const noexcept {
    return _platform.get();
}

non_owning_rptr<engine::ResourceManager> EditorEngine::getResources() const noexcept {
    return _resources.get();
}

non_owning_rptr<engine::Scheduler> EditorEngine::getScheduler() const noexcept {
    return _scheduler.get();
}

ref<GlobalEventEmitter> EditorEngine::getEmitter() const noexcept {
    return const_cast<ref<GlobalEventEmitter>>(_emitter);
}

ref<engine::core::Modules> EditorEngine::getModules() const noexcept {
    return const_cast<ref<engine::core::Modules>>(_modules);
}

Vector<non_owning_rptr<engine::core::WorldContext>> EditorEngine::getWorldContexts() const noexcept {
    return _worldContexts;
}

void EditorEngine::addWorld(cref<sptr<engine::core::World>> world_) {
    _emitter.emit<WorldAddedEvent>(world_);
}

void EditorEngine::removeWorld(cref<sptr<engine::core::World>> world_) {
    _emitter.emit<WorldRemoveEvent>(world_);
}

const non_owning_rptr<engine::core::Session> EditorEngine::getEditorSession() const noexcept {
    return _editorSession.get();
}

const non_owning_rptr<engine::core::Session> EditorEngine::getPrimaryGameSession() const noexcept {
    return _primaryGameSession.get();
}

void EditorEngine::setupCorePipelines() {
    auto corePipeline = make_uptr<schedule::CorePipeline>();
    _scheduler->getCompositePipeline()->addPipeline(_STD move(corePipeline));
}
