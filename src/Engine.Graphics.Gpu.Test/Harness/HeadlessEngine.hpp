#pragma once

#include <Engine.Graphics/Graphics.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
#endif

namespace hg::test::gpu {
	/**
	 * @details Engine with a headless graphics module only; constructing it makes it the current engine.
	 */
	class HeadlessEngine final :
		public engine::Engine {
	public:
		HeadlessEngine() = default;

		~HeadlessEngine() override = default;

	public:
		std::function<void(vk::Instance)> onInstance;
		std::function<void()> beforeInstance;

	private:
		mutable engine::Config _config;
		mutable GlobalEventEmitter _emitter;
		engine::core::Modules _modules;
		engine::core::Timing _timing;
		Vector<nmpt<engine::core::UniverseContext>> _universeContexts;

		// Declared last: it reaches the engine members above through `*this`
		mutable engine::Graphics _graphics { *this };

	public:
		bool preInit() override {
			return setEngineState(engine::core::EngineState::ePreInitialized);
		}

		bool init() override {
			_graphics.setupHeadless(onInstance);
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
			_graphics.destroyHeadless(beforeInstance);
			return setEngineState(engine::core::EngineState::eShutdown);
		}

		bool exit() override {
			return setEngineState(engine::core::EngineState::eExited);
		}

	public:
		[[nodiscard]] nmpt<engine::ActorModule> getActors() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Assets> getAssets() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Audio> getAudio() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<engine::Graphics> getGraphics() const noexcept override {
			return &_graphics;
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
			return nullptr;
		}

		nmpt<const engine::SerializationModule> getSerialization() const noexcept override {
			return nullptr;
		}

		[[nodiscard]] nmpt<const engine::StorageModule> getStorage() const noexcept override {
			return nullptr;
		}

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
		[[nodiscard]] std::span<const nmpt<engine::core::UniverseContext>> getUniverseContexts() const noexcept override {
			return _universeContexts;
		}

		void addUniverse([[maybe_unused]] mref<SharedPtr<engine::core::Universe>> universe_) override {}

		void removeUniverse([[maybe_unused]] mref<SharedPtr<engine::core::Universe>> universe_) override {}
	};
}
