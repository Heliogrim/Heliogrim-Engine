#pragma once

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Timing.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

namespace HeliogrimApi::test_engine {
	using namespace hg;

	/* Minimal engine: only the actor module is live, so the spawn API reaches a real registry. */
	class SpawnEngine final :
		public engine::Engine {
	public:
		SpawnEngine() :
			_actors(make_uptr<engine::ActorModule>(*this)) {
			_actors->setup();
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
		[[nodiscard]] nmpt<engine::ActorModule> getActors() const noexcept override { return _actors.get(); }

		[[nodiscard]] nmpt<engine::Assets> getAssets() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Audio> getAudio() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Graphics> getGraphics() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Input> getInput() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Network> getNetwork() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Physics> getPhysics() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Platform> getPlatform() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::ResourceManager> getResources() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<engine::Scheduler> getScheduler() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<const engine::SerializationModule> getSerialization() const noexcept override { return nullptr; }

		[[nodiscard]] nmpt<const engine::StorageModule> getStorage() const noexcept override { return nullptr; }

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
