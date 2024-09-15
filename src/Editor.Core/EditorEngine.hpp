#pragma once

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

/**/
#include <Engine.Storage/StorageModule.hpp>
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
		engine::StorageModule _storage;

		engine::Config _config;
		GlobalEventEmitter _emitter;

		engine::core::Modules _modules;

	public:
		[[nodiscard]] nmpt<engine::ActorModule> getActors() const noexcept override;

		[[nodiscard]] nmpt<engine::Assets> getAssets() const noexcept override;

		[[nodiscard]] nmpt<engine::Audio> getAudio() const noexcept override;

		[[nodiscard]] nmpt<engine::Graphics> getGraphics() const noexcept override;

		[[nodiscard]] nmpt<engine::Input> getInput() const noexcept override;

		[[nodiscard]] nmpt<engine::Network> getNetwork() const noexcept override;

		[[nodiscard]] nmpt<engine::Physics> getPhysics() const noexcept override;

	public:
		[[nodiscard]] nmpt<engine::Platform> getPlatform() const noexcept override;

		[[nodiscard]] nmpt<engine::ResourceManager> getResources() const noexcept override;

		[[nodiscard]] nmpt<engine::Scheduler> getScheduler() const noexcept override;

		[[nodiscard]] nmpt<const engine::StorageModule> getStorage() const noexcept override;

	public:
		[[nodiscard]] ref<engine::Config> getConfig() const noexcept override;

		[[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override;

		[[nodiscard]] ref<engine::core::Modules> getModules() const noexcept override;

	private:
		Array<nmpt<engine::core::UniverseContext>, 2> _universeContexts;

	public:
		[[nodiscard]] std::span<const nmpt<engine::core::UniverseContext>>
		getUniverseContexts() const noexcept override;

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
