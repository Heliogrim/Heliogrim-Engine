#include "GameEngine.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Event/UniverseAddedEvent.hpp>
#include <Engine.Core/Event/UniverseRemoveEvent.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Core/EngineState.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Helper/Wait.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.SFX/Audio.hpp>

#ifdef WIN32
#include <Support.Platform.Win32/WinPlatform.hpp>
#else
//#include <Engine.Platform/Linux>
#endif

using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

GameEngine::GameEngine() :
	_storage(*this) {}

GameEngine::~GameEngine() = default;

bool GameEngine::preInit() {
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

	/* Register Root Modules */
	_modules.addRootModule(*_platform);
	_modules.addRootModule(*_resources);
	_modules.addRootModule(*_scheduler);
	_modules.addRootModule(_storage);

	/**/

	_assets = make_uptr<Assets>(*this);
	_audio = make_uptr<Audio>(*this);
	_graphics = make_uptr<Graphics>(*this);
	_input = make_uptr<Input>(*this);
	_network = make_uptr<Network>(*this);
	_physics = make_uptr<Physics>(*this);

	/* Register Core Modules */
	_modules.addCoreModule(*_assets, AssetsDepKey);
	_modules.addCoreModule(*_audio, AudioDepKey);
	_modules.addCoreModule(*_graphics, GraphicsDepKey);
	_modules.addCoreModule(*_input, InputDepKey);
	_modules.addCoreModule(*_network, NetworkDepKey);
	_modules.addCoreModule(*_physics, PhysicsDepKey);

	return setEngineState(EngineState::ePreInitialized);
}

bool GameEngine::init() {
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
	std::atomic_uint_fast8_t setupCounter { 0u };

	scheduler::exec(
		[this, &setupCounter] {
			_assets->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	scheduler::exec(
		[this, &setupCounter] {
			_audio->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	scheduler::exec(
		[this, &setupCounter] {
			_graphics->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	scheduler::exec(
		[this, &setupCounter] {
			_input->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	scheduler::exec(
		[this, &setupCounter] {
			_network->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	scheduler::exec(
		[this, &setupCounter] {
			_physics->setup();
			++setupCounter;
			setupCounter.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(setupCounter, 6u);
	setupCounter.store(0u, std::memory_order::relaxed);

	scheduler::exec(
		[this, &setupCounter] {

			for (const auto& subModule : _modules.getSubModules()) {
				subModule->setup();
			}

			++setupCounter;
			setupCounter.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(setupCounter, static_cast<u8>(_modules.getSubModuleCount()));
	return setEngineState(EngineState::eInitialized);
}

bool GameEngine::postInit() {
	if (getEngineState() != EngineState::eInitialized) {
		return false;
	}

	/**/
	_scheduler->finalize();

	// TODO: Asset seeder resolving
	// TODO:

	return setEngineState(EngineState::ePostInitialized);
}

bool GameEngine::start() {
	if (getEngineState() != EngineState::ePostInitialized) {
		return false;
	}

	/**/
	std::atomic_flag next {};
	scheduler::exec(
		[this, &next] {
			_scheduler->getCompositePipeline()->start();

			next.test_and_set(std::memory_order_relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	next.clear(std::memory_order::release);

	/**/
	scheduler::exec(
		[this, &next] {
			_assets->start();
			_audio->start();
			_graphics->start();
			_input->start();
			_network->start();
			_physics->start();

			next.test_and_set(std::memory_order::relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	next.clear(std::memory_order::release);

	/**/
	scheduler::exec(
		[this, &next] {
			_gameSession = make_uptr<core::Session>();
			_universeContexts.emplace_back(std::addressof(_gameSession->getUniverseContext()));

			/**/

			for (const auto& subModule : _modules.getSubModules()) {
				subModule->start();
			}

			/**/

			next.test_and_set(std::memory_order::relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	return setEngineState(EngineState::eStarted);
}

bool GameEngine::stop() {
	if (getEngineState() != EngineState::eStarted) {
		return false;
	}

	/**/
	std::atomic_flag next {};
	scheduler::exec(
		[this, &next] {

			for (const auto& subModule : _modules.getSubModules()) {
				subModule->stop();
			}

			/**/

			auto where = std::ranges::remove(
				_universeContexts,
				nmpt<core::UniverseContext> { std::addressof(_gameSession->getUniverseContext()) }
			);
			_universeContexts.erase(where.begin(), where.end());
			_gameSession.reset();

			/**/

			next.test_and_set(std::memory_order::relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	next.clear(std::memory_order::release);

	/**/
	scheduler::exec(
		[this, &next] {
			_physics->stop();
			_network->stop();
			_input->stop();
			_graphics->stop();
			_audio->stop();
			_assets->stop();

			next.test_and_set(std::memory_order::relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	next.clear(std::memory_order::release);

	/**/
	scheduler::exec(
		[this, &next] {
			_scheduler->getCompositePipeline()->stop();

			next.test_and_set(std::memory_order_relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	return setEngineState(EngineState::eStopped);
}

bool GameEngine::shutdown() {
	if (getEngineState() != EngineState::eStopped && getEngineState() != EngineState::eInitialized) {
		return false;
	}

	std::atomic_flag subModuleFlag {};
	scheduler::exec(
		[this, &subModuleFlag] {

			const auto& modules = _modules.getSubModules();
			for (auto iter = modules.rbegin(); iter != modules.rend(); ++iter) {
				(*iter)->destroy();
			}

			subModuleFlag.test_and_set(std::memory_order::relaxed);
			subModuleFlag.notify_one();
		}
	);

	scheduler::waitOnAtomic(subModuleFlag, false);

	/* Core modules should always interact with a guaranteed fiber context and non-sequential execution */
	std::atomic_uint_fast8_t moduleCount { 0u };

	scheduler::exec(
		[this, &moduleCount] {
			_physics->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	scheduler::exec(
		[this, &moduleCount] {
			_network->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	scheduler::exec(
		[this, &moduleCount] {
			_input->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	scheduler::exec(
		[this, &moduleCount] {
			_graphics->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	scheduler::exec(
		[this, &moduleCount] {
			_audio->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	scheduler::exec(
		[this, &moduleCount] {
			_assets->destroy();
			++moduleCount;
			moduleCount.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(moduleCount, 6u);

	/* Base module are setup via direct call without fiber context guarantee (which is unlikely) */
	_resources->destroy();
	_scheduler->destroy();
	_platform->tidy();

	return setEngineState(EngineState::eShutdown);
}

bool GameEngine::exit() {
	if (getEngineState() != EngineState::eShutdown) {
		return false;
	}

	/* Unregister Core Modules */
	_modules.removeCoreModule(*_physics);
	_modules.removeCoreModule(*_network);
	_modules.removeCoreModule(*_input);
	_modules.removeCoreModule(*_graphics);
	_modules.removeCoreModule(*_audio);
	_modules.removeCoreModule(*_assets);

	/**/
	_physics.reset();
	_network.reset();
	_input.reset();
	_graphics.reset();
	_audio.reset();
	_assets.reset();

	/* Unregister Root Modules */
	_modules.removeRootModule(_storage);
	_modules.removeRootModule(*_scheduler);
	_modules.removeRootModule(*_resources);
	_modules.removeRootModule(*_platform);

	/**/
	_resources.reset();
	_scheduler.reset();
	_platform.reset();

	return setEngineState(EngineState::eExited);
}

nmpt<Assets> GameEngine::getAssets() const noexcept {
	return _assets.get();
}

nmpt<Audio> GameEngine::getAudio() const noexcept {
	return _audio.get();
}

nmpt<Graphics> GameEngine::getGraphics() const noexcept {
	return _graphics.get();
}

nmpt<Input> GameEngine::getInput() const noexcept {
	return _input.get();
}

nmpt<Network> GameEngine::getNetwork() const noexcept {
	return _network.get();
}

nmpt<Physics> GameEngine::getPhysics() const noexcept {
	return _physics.get();
}

nmpt<Platform> GameEngine::getPlatform() const noexcept {
	return _platform.get();
}

nmpt<ResourceManager> GameEngine::getResources() const noexcept {
	return _resources.get();
}

nmpt<Scheduler> GameEngine::getScheduler() const noexcept {
	return _scheduler.get();
}

nmpt<const StorageModule> GameEngine::getStorage() const noexcept {
	return std::addressof(_storage);
}

ref<Config> GameEngine::getConfig() const noexcept {
	return const_cast<ref<engine::Config>>(_config);
}

ref<GlobalEventEmitter> GameEngine::getEmitter() const noexcept {
	return const_cast<ref<GlobalEventEmitter>>(_emitter);
}

ref<core::Modules> GameEngine::getModules() const noexcept {
	return const_cast<ref<core::Modules>>(_modules);
}

Vector<nmpt<core::UniverseContext>> GameEngine::getUniverseContexts() const noexcept {
	return _universeContexts;
}

void GameEngine::addUniverse(cref<sptr<core::Universe>> universe_) {
	_emitter.emit<UniverseAddedEvent>(universe_);
}

void GameEngine::removeUniverse(cref<sptr<core::Universe>> universe_) {
	_emitter.emit<UniverseRemoveEvent>(universe_);
}

nmpt<core::Session> GameEngine::getGameSession() const noexcept {
	return _gameSession.get();
}

void GameEngine::setupCorePipelines() {
	auto corePipeline = make_uptr<schedule::CorePipeline>();
	_scheduler->getCompositePipeline()->addPipeline(std::move(corePipeline));
}
