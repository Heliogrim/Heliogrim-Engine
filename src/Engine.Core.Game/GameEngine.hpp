#pragma once

/**/

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
import Engine.Storage;
#endif
/**/

namespace hg::engine {
	class GameEngine :
		public InheritMeta<GameEngine, Engine> {
	public:
		GameEngine();

		~GameEngine() override;

	public:
		bool preInit() override;

		bool init() override;

		bool postInit() override;

		bool start() override;

		bool stop() override;

		bool shutdown() override;

		bool exit() override;

	private:
		/* Core Module */

		uptr<ActorModule> _actors;
		uptr<Assets> _assets;
		uptr<Audio> _audio;
		uptr<Graphics> _graphics;
		uptr<Input> _input;
		uptr<Network> _network;
		uptr<Physics> _physics;

		/* Root Modules */

		uptr<Platform> _platform;
		uptr<ResourceManager> _resources;
		uptr<Scheduler> _scheduler;
		SerializationModule _serialization;
		StorageModule _storage;

		/**/

		Config _config;
		GlobalEventEmitter _emitter;
		core::Modules _modules;
		core::Timing _timing;

	public:
		[[nodiscard]] nmpt<ActorModule> getActors() const noexcept override;

		[[nodiscard]] nmpt<Assets> getAssets() const noexcept override;

		[[nodiscard]] nmpt<Audio> getAudio() const noexcept override;

		[[nodiscard]] nmpt<Graphics> getGraphics() const noexcept override;

		[[nodiscard]] nmpt<Input> getInput() const noexcept override;

		[[nodiscard]] nmpt<Network> getNetwork() const noexcept override;

		[[nodiscard]] nmpt<Physics> getPhysics() const noexcept override;

	public:
		[[nodiscard]] nmpt<Platform> getPlatform() const noexcept override;

		[[nodiscard]] nmpt<ResourceManager> getResources() const noexcept override;

		[[nodiscard]] nmpt<Scheduler> getScheduler() const noexcept override;

		[[nodiscard]] nmpt<const SerializationModule> getSerialization() const noexcept override;

		[[nodiscard]] nmpt<const StorageModule> getStorage() const noexcept override;

	public:
		[[nodiscard]] ref<Config> getConfig() const noexcept override;

		[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override;

		[[nodiscard]] ref<core::Modules> getModules() const noexcept override;

		[[nodiscard]] ref<core::Timing> getTiming() const noexcept override;

	private:
		Array<nmpt<core::UniverseContext>, 1> _universeContexts;

	public:
		[[nodiscard]] std::span<const nmpt<core::UniverseContext>> getUniverseContexts() const noexcept override;

	public:
		void addUniverse(mref<SharedPtr<core::Universe>> universe_) override;

		void removeUniverse(mref<SharedPtr<core::Universe>> universe_) override;

	private:
		/**
		 * GameEngine specific member functions and attributes
		 */

		/**
		 * As long as we use a GameEngine instance we can expect to have at least one major universe
		 *  We still have the possibility to execute multiple game sessions at once, but this should
		 *  be managed explicitly to model the interaction of multiple universes.
		 */
		uptr<core::Session> _gameSession;

	public:
		[[nodiscard]] nmpt<core::Session> getGameSession() const noexcept;

	private:
		void setupCorePipelines();
	};
}
