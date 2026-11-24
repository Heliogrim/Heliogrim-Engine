#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
import Engine.Serialization;
#endif

namespace HeliogrimApi::test_engine {
	using namespace hg;

	/* Minimal engine: only the actor module is live, so the spawn API reaches a real registry. */
	class SpawnEngine final :
		public engine::Engine {
	public:
		SpawnEngine() :
			_actors(make_uptr<engine::ActorModule>(*this)) {
			_actors->setup();
			_modules.addCoreModule(*_actors, engine::core::ActorDepKey);
			_modules.addRootModule(*_serialization);
		}

		~SpawnEngine() override = default;

	private:
		uptr<engine::ActorModule> _actors;

	public:
		bool preInit() override { return true; }

		bool init() override { return true; }

		bool postInit() override { return true; }

		bool start() override { return true; }

		bool stop() override { return true; }

		bool shutdown() override { return true; }

		bool exit() override { return true; }

	public:
		/* Empty scheme registry; tests register the schemes they load through. */
		uptr<engine::SerializationModule> _serialization = make_uptr<engine::SerializationModule>();

	private:
		mutable engine::Config _config;
		mutable GlobalEventEmitter _emitter;
		engine::core::Modules _modules;
		engine::core::Timing _timing;
		Vector<nmpt<engine::core::UniverseContext>> _universeContexts;

	public:
		[[nodiscard]] ref<engine::Config> getConfig() const noexcept override { return _config; }

		[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override { return _emitter; }

		[[nodiscard]] ref<engine::core::Modules> getModules() const noexcept override {
			return const_cast<ref<engine::core::Modules>>(_modules);
		}

		[[nodiscard]] ref<engine::core::Timing> getTiming() const noexcept override {
			return const_cast<ref<engine::core::Timing>>(_timing);
		}

		[[nodiscard]] std::span<const nmpt<engine::core::UniverseContext>> getUniverseContexts() const noexcept override {
			return _universeContexts;
		}

		void addUniverse(mref<SharedPtr<engine::core::Universe>>) override {}

		void removeUniverse(mref<SharedPtr<engine::core::Universe>>) override {}
	};
}
