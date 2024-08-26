#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/EngineState.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

namespace hg::test {
	class TestSchedulerEngine final :
		public engine::Engine {
	public:
		TestSchedulerEngine() = default;

		~TestSchedulerEngine() override = default;

	private:
		uptr<engine::Scheduler> _scheduler;

	public:
		bool preInit() override {
			_scheduler = make_uptr<engine::CompScheduler>();
			return setEngineState(engine::core::EngineState::ePreInitialized);
		}

		bool init() override {
			_scheduler->setup(engine::Scheduler::auto_worker_count);
			return setEngineState(engine::core::EngineState::eInitialized);
		}

		bool postInit() override {
			return setEngineState(engine::core::EngineState::ePostInitialized);
		}

		bool start() override {
			return setEngineState(engine::core::EngineState::eStarted);
		}

		bool stop() override {
			return setEngineState(engine::core::EngineState::eStopped);
		}

		bool shutdown() override {
			return setEngineState(engine::core::EngineState::eShutdown);
		}

		bool exit() override {
			_scheduler.reset();
			return setEngineState(engine::core::EngineState::eExited);
		}

	public:
		[[nodiscard]] nmpt<engine::Assets> getAssets() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Audio> getAudio() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Graphics> getGraphics() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Input> getInput() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Network> getNetwork() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Physics> getPhysics() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Platform> getPlatform() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::ResourceManager> getResources() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Scheduler> getScheduler() const noexcept override {
			return _scheduler.get();
		}

		[[nodiscard]] nmpt<const engine::StorageModule> getStorage() const noexcept override {
			return nullptr;
		}

	private:
		mutable engine::Config _config;
		mutable GlobalEventEmitter _emitter;
		engine::core::Modules _modules;

	public:
		[[nodiscard]] ref<engine::Config> getConfig() const noexcept override {
			return _config;
		}

		[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override {
			return _emitter;
		}

		[[nodiscard]] ref<engine::core::Modules> getModules() const noexcept override {
			return const_cast<ref<engine::core::Modules>>(_modules);
		}

	private:
		Vector<nmpt<engine::core::UniverseContext>> _universeContexts;

		[[nodiscard]] Vector<nmpt<engine::core::UniverseContext>> getUniverseContexts() const noexcept override {
			return _universeContexts;
		}

		void addUniverse(cref<sptr<engine::core::Universe>> universe_) override {}

		void removeUniverse(cref<sptr<engine::core::Universe>> universe_) override {}
	};
}
