#include "EditorEngine.hpp"

#include <algorithm>
#include <ranges>
#include <Editor.Boot/Boot/ConfigInit.hpp>
#include <Editor.Boot/Boot/SerializationInit.hpp>
#include <Editor.Boot/Boot/StorageInit.hpp>
#include <Editor.Boot/Boot/SubModuleInit.hpp>
#include <Engine.ACS.Schedule/ActorPipeline.hpp>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Config/Provider/EditorProvider.hpp>
#include <Engine.Config/Provider/ProjectProvider.hpp>
#include <Engine.Config/Provider/SystemProvider.hpp>
#include <Engine.Core.Schedule/CorePipeline.hpp>
#include <Engine.Core/EngineState.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Core/Event/UniverseAddedEvent.hpp>
#include <Engine.Core/Event/UniverseRemoveEvent.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene.Schedule/ScenePipeline.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Helper/Wait.hpp>
#include <Engine.SFX/Audio.hpp>

#ifdef WIN32
#include <Support.Platform.Win32/WinPlatform.hpp>
#else
#include <Engine.Platform/Platform.hpp>
//#include <Engine.Platform/Linux>
#endif

using namespace hg::editor;
using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

EditorEngine::EditorEngine() :
	_storage(*this),
	_config(make_uptr<engine::cfg::SystemProvider>()),
	_universeContexts() {}

EditorEngine::~EditorEngine() = default;

nmpt<EditorEngine> EditorEngine::getEngine() noexcept {
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

	/**/

	// Note: Changing order will effect query lookup priority, which may change value reports and impact performance.
	auto& editorConfigProvider = _config.registerProvider(make_uptr<cfg::EditorProvider>());
	auto& projectConfigProvider = _config.registerProvider(make_uptr<engine::cfg::ProjectProvider>());

	boot::initEditorConfig(_config, editorConfigProvider);
	boot::initProjectConfig(_config, projectConfigProvider);

	/**/

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
	_modules.addRootModule(_serialization);
	_modules.addRootModule(_storage);

	/**/

	_actors = make_uptr<ActorModule>(*this);
	_assets = make_uptr<Assets>(*this);
	_audio = make_uptr<Audio>(*this);
	_graphics = make_uptr<Graphics>(*this);
	_input = make_uptr<Input>(*this);
	_network = make_uptr<Network>(*this);
	_physics = make_uptr<Physics>(*this);

	/* Register Core Modules */
	_modules.addCoreModule(*_actors, ActorDepKey);
	_modules.addCoreModule(*_assets, AssetsDepKey);
	_modules.addCoreModule(*_audio, AudioDepKey);
	_modules.addCoreModule(*_graphics, GraphicsDepKey);
	_modules.addCoreModule(*_input, InputDepKey);
	_modules.addCoreModule(*_network, NetworkDepKey);
	_modules.addCoreModule(*_physics, PhysicsDepKey);

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

	/* Note: Root modules are setup via direct call without fiber context guarantee. */
	_platform->setup();
	_scheduler->setup(Scheduler::auto_worker_count);
	_storage.setup(_config);
	_resources->setup();

	/**/

	_editorSession = make_uptr<core::Session>();
	_universeContexts.front() = std::addressof(_editorSession->getUniverseContext());

	_primaryGameSession = make_uptr<core::Session>();
	_universeContexts.back() = std::addressof(_primaryGameSession->getUniverseContext());

	/**/

	boot::initSerialization(_config, _serialization);
	boot::initStorage(_config, _storage);
	setupCorePipelines();

	/* Core modules should always interact with a guaranteed fiber context and non-sequential execution */
	std::atomic_uint_fast8_t setupCounter { 0u };
	_modules.forEachCoreModule(
		[&setupCounter](ref<CoreModule> module_) {
			scheduler::exec(
				[coreModule = std::addressof(module_), &setupCounter] {
					coreModule->setup();
					++setupCounter;
					setupCounter.notify_one();
				}
			);
		}
	);

	/**/
	scheduler::waitUntilAtomic(setupCounter, _modules.getCoreModuleCount());
	setupCounter.store(0u, std::memory_order_relaxed);

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
	scheduler::waitUntilAtomic(setupCounter, 1u);
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
			_modules.forEachCoreModule(
				[](ref<CoreModule> module_) {
					module_.start();
				}
			);

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

			for (const auto& subModule : _modules.getSubModules()) {
				subModule->start();
			}

			next.test_and_set(std::memory_order::relaxed);
			next.notify_one();
		}
	);

	/**/
	scheduler::waitUntilAtomic(next, true);
	return setEngineState(EngineState::eStarted);
}

bool EditorEngine::stop() {
	if (getEngineState() != EngineState::eStarted) {
		return false;
	}

	/**/
	std::atomic_flag next {};
	scheduler::exec(
		[this, &next] {

			const auto& modules = _modules.getSubModules();
			for (auto iter = modules.rbegin(); iter != modules.rend(); ++iter) {
				(*iter)->stop();
			}

			/**/

			auto prevEditorUniverse = _editorSession->getUniverseContext().getCurrentUniverse();
			_editorSession->getUniverseContext().setCurrentUniverse(nullptr);
			removeUniverse(clone(prevEditorUniverse));
			prevEditorUniverse.reset();

			/**/

			auto prevPrimaryUniverse = _primaryGameSession->getUniverseContext().getCurrentUniverse();
			_primaryGameSession->getUniverseContext().setCurrentUniverse(nullptr);
			removeUniverse(clone(prevPrimaryUniverse));
			prevPrimaryUniverse.reset();

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
			_modules.forEachCoreModule(
				[](ref<CoreModule> module_) {
					module_.stop();
				}
			);

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

bool EditorEngine::shutdown() {
	if (getEngineState() != EngineState::eStopped && getEngineState() != EngineState::eInitialized) {
		return false;
	}

	std::atomic_flag subModuleFlag {};
	scheduler::exec(
		[this, &subModuleFlag] {

			const auto& modules = _modules.getSubModules();
			while (not modules.empty()) {
				auto& module = modules.back();
				module->destroy();
				_modules.removeSubModule(module.get());
			}

			subModuleFlag.test_and_set(std::memory_order_relaxed);
			subModuleFlag.notify_one();
		}
	);

	scheduler::waitOnAtomic(subModuleFlag, false);

	/* Core modules should always interact with a guaranteed fiber context and non-sequential execution */
	std::atomic_uint_fast8_t moduleCount { 0u };
	_modules.forEachCoreModule(
		[&moduleCount](ref<CoreModule> module_) {
			scheduler::exec(
				[coreModule = std::addressof(module_), &moduleCount] {
					coreModule->destroy();
					++moduleCount;
					moduleCount.notify_one();
				}
			);
		}
	);

	/**/
	scheduler::waitUntilAtomic(moduleCount, _modules.getCoreModuleCount());

	/**/
	_universeContexts.back() = nullptr;
	_primaryGameSession.reset();

	_universeContexts.front() = nullptr;
	_editorSession.reset();

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

	/* Unregister Core Modules */
	_modules.removeCoreModule(*_physics);
	_modules.removeCoreModule(*_network);
	_modules.removeCoreModule(*_input);
	_modules.removeCoreModule(*_graphics);
	_modules.removeCoreModule(*_audio);
	_modules.removeCoreModule(*_assets);
	_modules.removeCoreModule(*_actors);

	/**/
	_physics.reset();
	_network.reset();
	_input.reset();
	_graphics.reset();
	_audio.reset();
	_assets.reset();
	_actors.reset();

	/* Unregister Root Modules */
	_modules.removeRootModule(_storage);
	_modules.removeRootModule(_serialization);
	_modules.removeRootModule(*_scheduler);
	_modules.removeRootModule(*_resources);
	_modules.removeRootModule(*_platform);

	/**/
	_platform.reset();
	_scheduler.reset();
	_resources.reset();

	return setEngineState(EngineState::eExited);
}

nmpt<engine::ActorModule> EditorEngine::getActors() const noexcept {
	return _actors.get();
}

nmpt<engine::Assets> EditorEngine::getAssets() const noexcept {
	return _assets.get();
}

nmpt<engine::Audio> EditorEngine::getAudio() const noexcept {
	return _audio.get();
}

nmpt<engine::Graphics> EditorEngine::getGraphics() const noexcept {
	return _graphics.get();
}

nmpt<engine::Input> EditorEngine::getInput() const noexcept {
	return _input.get();
}

nmpt<engine::Network> EditorEngine::getNetwork() const noexcept {
	return _network.get();
}

nmpt<engine::Physics> EditorEngine::getPhysics() const noexcept {
	return _physics.get();
}

nmpt<engine::Platform> EditorEngine::getPlatform() const noexcept {
	return _platform.get();
}

nmpt<engine::ResourceManager> EditorEngine::getResources() const noexcept {
	return _resources.get();
}

nmpt<engine::Scheduler> EditorEngine::getScheduler() const noexcept {
	return _scheduler.get();
}

nmpt<const engine::SerializationModule> EditorEngine::getSerialization() const noexcept {
	return std::addressof(_serialization);
}

nmpt<const engine::StorageModule> EditorEngine::getStorage() const noexcept {
	return std::addressof(_storage);
}

ref<engine::Config> EditorEngine::getConfig() const noexcept {
	return const_cast<ref<engine::Config>>(_config);
}

ref<GlobalEventEmitter> EditorEngine::getEmitter() const noexcept {
	return const_cast<ref<GlobalEventEmitter>>(_emitter);
}

ref<engine::core::Modules> EditorEngine::getModules() const noexcept {
	return const_cast<ref<engine::core::Modules>>(_modules);
}

ref<engine::core::Timing> EditorEngine::getTiming() const noexcept {
	return const_cast<ref<engine::core::Timing>>(_timing);
}

std::span<const nmpt<engine::core::UniverseContext>> EditorEngine::getUniverseContexts() const noexcept {
	return std::span { _universeContexts };
}

void EditorEngine::addUniverse(mref<SharedPtr<engine::core::Universe>> universe_) {
	_emitter.emit<UniverseAddedEvent>(std::move(universe_));
}

void EditorEngine::removeUniverse(mref<SharedPtr<engine::core::Universe>> universe_) {
	_emitter.emit<UniverseRemoveEvent>(std::move(universe_));
}

nmpt<engine::core::Session> EditorEngine::getEditorSession() const noexcept {
	return _editorSession.get();
}

nmpt<engine::core::Session> EditorEngine::getPrimaryGameSession() const noexcept {
	return _primaryGameSession.get();
}

void EditorEngine::setupCorePipelines() {

	auto corePipeline = make_uptr<schedule::CorePipeline>();
	auto actorPipeline = make_uptr<acs::schedule::ActorPipeline>();
	auto scenePipeline = make_uptr<scene::schedule::ScenePipeline>();

	/**/

	_scheduler->getCompositePipeline()->addPipeline(std::move(corePipeline));
	_scheduler->getCompositePipeline()->addPipeline(std::move(actorPipeline));
	_scheduler->getCompositePipeline()->addPipeline(std::move(scenePipeline));
}
