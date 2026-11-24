#pragma once

#include "TestCompScheduler.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
import Engine.Scheduler;
#endif

namespace hg::test {
	class TestSchedulerEngine final :
		public engine::Engine {
	public:
		TestSchedulerEngine() = default;

		explicit TestSchedulerEngine(const u32 workers_) :
			_workers(workers_) {}

		TestSchedulerEngine(const u32 workers_, cref<engine::scheduler::LanePolicyConfig> lanes_) :
			_workers(workers_),
			_lanes(lanes_) {}

		~TestSchedulerEngine() override = default;

	private:
		uptr<engine::Scheduler> _scheduler;
		u32 _workers = engine::Scheduler::auto_worker_count;
		engine::scheduler::LanePolicyConfig _lanes {};

	public:
		bool preInit() override {
			_scheduler = make_uptr<engine::TestCompScheduler>();
			_modules.addRootModule(*_scheduler);
			return setEngineState(engine::core::EngineState::ePreInitialized);
		}

		bool init() override {
			_scheduler->setup(_workers, engine::Scheduler::default_worker_reserve, _lanes);
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
			if (_scheduler != nullptr) {
				_modules.removeRootModule(*_scheduler);
			}
			_scheduler.reset();
			return setEngineState(engine::core::EngineState::eExited);
		}

	private:
		mutable engine::Config _config;
		mutable GlobalEventEmitter _emitter;
		engine::core::Modules _modules;
		engine::core::Timing _timing;

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

		[[nodiscard]] ref<engine::core::Timing> getTiming() const noexcept override {
			return const_cast<ref<engine::core::Timing>>(_timing);
		}

	private:
		Vector<nmpt<engine::core::UniverseContext>> _universeContexts;

		[[nodiscard]] std::span<const nmpt<engine::core::UniverseContext>> getUniverseContexts() const noexcept override {
			return _universeContexts;
		}

		void addUniverse([[maybe_unused]] mref<SharedPtr<engine::core::Universe>> universe_) override {}

		void removeUniverse([[maybe_unused]] mref<SharedPtr<engine::core::Universe>> universe_) override {}
	};
}

namespace SchedulerModule {
	inline void structuredRuntimeTest(
		const hg::u32 workers_,
		hg::cref<hg::engine::scheduler::LanePolicyConfig> lanes_,
		std::function<void(hg::nmpt<hg::engine::Scheduler>)>&& callback_
	) {

		auto engine = hg::make_uptr<hg::test::TestSchedulerEngine>(workers_, lanes_);
		engine->preInit();

		const ::hg::nmpt<::hg::engine::Scheduler> scheduler = std::addressof(*::hg::engine::Scheduler::get());

		//
		EXPECT_EQ(scheduler->getWorkerCount(), 0uL);

		//
		engine->init();
		EXPECT_NE(scheduler->getWorkerCount(), 0uL);

		//
		engine->postInit();
		engine->start();

		//
		if (callback_) {
			std::forward<decltype(callback_)>(callback_)(scheduler);
		}

		//
		engine->stop();
		engine->shutdown();
		engine->exit();

		//
		engine.reset();
	}

	inline void structuredRuntimeTest(
		const hg::u32 workers_,
		std::function<void(hg::nmpt<hg::engine::Scheduler>)>&& callback_
	) {
		structuredRuntimeTest(workers_, hg::engine::scheduler::LanePolicyConfig {}, std::move(callback_));
	}

	inline void structuredRuntimeTest(std::function<void(hg::nmpt<hg::engine::Scheduler>)>&& callback_) {
		structuredRuntimeTest(hg::engine::Scheduler::auto_worker_count, std::move(callback_));
	}
}
