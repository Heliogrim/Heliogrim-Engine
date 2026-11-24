#pragma once

/**/

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Assets;
import Engine.Audio;
import Engine.Common;
import Engine.Config;
import Engine.Core;
import Engine.Event;
import Engine.Graphics.Core;
import Engine.Input;
import Engine.Network;
import Engine.Physics;
import Engine.Platform;
import Engine.Resource;
import Engine.Scheduler;
import Engine.Storage;
#endif
/**/

namespace hg::editor {
	class EditorEngine :
		public InheritMeta<EditorEngine, ::hg::engine::Engine> {
	public:
		EditorEngine();

		~EditorEngine() override;

	public:
		/**
		 * Get the cached engine instance pointer
		 */
		[[nodiscard]] static nmpt<EditorEngine> getEngine() noexcept;

	public:
		bool preInit() override;

		bool init() override;

		bool postInit() override;

		bool start() override;

		bool stop() override;

		bool shutdown() override;

		bool exit() override;

	private:
		uptr<engine::ActorModule> _actors;
		uptr<engine::Assets> _assets;
		uptr<engine::Audio> _audio;
		uptr<engine::Graphics> _graphics;
		uptr<engine::Input> _input;
		uptr<engine::Network> _network;
		uptr<engine::Physics> _physics;

		uptr<engine::Platform> _platform;
		uptr<engine::ResourceManager> _resources;
		uptr<engine::Scheduler> _scheduler;
		engine::SerializationModule _serialization;
		engine::StorageModule _storage;

		engine::Config _config;
		GlobalEventEmitter _emitter;

		engine::core::Modules _modules;
		engine::core::Timing _timing;

	public:
		[[nodiscard]] ref<engine::Config> getConfig() const noexcept override;

		[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override;

		[[nodiscard]] ref<engine::core::Modules> getModules() const noexcept override;

		[[nodiscard]] ref<engine::core::Timing> getTiming() const noexcept override;

	private:
		Array<nmpt<engine::core::UniverseContext>, 2> _universeContexts;

	public:
		[[nodiscard]] std::span<const nmpt<engine::core::UniverseContext>> getUniverseContexts() const noexcept override;

	public:
		void addUniverse(mref<SharedPtr<engine::core::Universe>> universe_) override;

		void removeUniverse(mref<SharedPtr<engine::core::Universe>> universe_) override;

	private:
		/**
		 * EditorEngine specific member function and attributes
		 */

		/**
		 * While using a EditorEngine instance we expect to hold more than one game session at once, but there is a minimal setup to suggest.
		 *  So we will be holding the editor's session and one played / edited / addressed session to differ between the systems and encapsulate resources.
		 */
		uptr<engine::core::Session> _editorSession;
		uptr<engine::core::Session> _primaryGameSession;

	public:
		[[nodiscard]] nmpt<engine::core::Session> getEditorSession() const noexcept;

		[[nodiscard]] nmpt<engine::core::Session> getPrimaryGameSession() const noexcept;

	private:
		void setupCorePipelines();
	};
}
