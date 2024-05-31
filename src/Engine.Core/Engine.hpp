#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg {
	class GlobalEventEmitter;
}

namespace hg::engine {
	class Assets;
	class Audio;
	class Graphics;
	class Input;
	class Network;
	class Physics;

	class ResourceManager;
	class Scheduler;
	class Platform;

	class Config;
}

namespace hg::engine::core {
	enum class EngineState : u8;

	class Modules;

	class Session;
	class SessionState;

	class World;
	class WorldContext;
}

namespace hg::engine {
	class __declspec(novtable) Engine :
		public InheritBase<Engine> {
	public:
		using this_type = Engine;

	protected:
		Engine();

	public:
		virtual ~Engine();

	protected:
		/**
		 * The cached engine instance pointer
		 *  This is used for more easier access due to unknown or ambiguous entry points
		 */
		static ptr<Engine> _cached;

	public:
		/**
		 * Get the cached engine instance pointer
		 */
		[[nodiscard]] static nmpt<Engine> getEngine() noexcept;

	private:
		/**
		 * Privately scoped tidy function to enforce lifecycle based destruction
		 */
		void tidy();

	private:
		/*
		 * Engine Scope Lifecycle:
		 *
		 * > Nil
		 *      > preInit
		 *      | - core modules
		 *      | - independent resource pointer
		 *      | - out of order creation
		 *          > init
		 *          | - resolved module references
		 *          | - resolved dependencies
		 *          | - order dependent creation
		 *          | - sub-module instantiation
		 *          | - primary function invocation (e.g. `Scheduler Loop`, `Resource Databases`)
		 *              > postInit
		 *              | - dispatch of generative tasks
		 *              |   - create default generative resources (e.g. `BRDF Lut`, `Skybox Texture`, `Pseudo Directories`)
		 *              | - dispatch of loading tasks
		 *              |   - resource indexing
		 *              |   - default resource loading (e.g. `Default Textures`, `User Preferences`)
		 *              | - dispatch of streaming tasks
		 *              |   - create stream caches
		 *              |   - preparation of stream archives to fast-load resources (e.g. `Serialized Scenes`, `Asset Resources`)
		 *                  <> start
		 *                  | - schedule main logic
		 *                  |   - schedule main graphics loop
		 *                  |   - schedule main audio loop
		 *                  |   - schedule main physics loop
		 *                  |   - schedule main input loop
		 *                  | - register / capture / activate system listeners
		 *                  |   - interrupt listeners
		 *                  |   - network event listeners
		 *                  | - enqueue user logic
		 *                  |   - dispatch actor based logic
		 *                  |   - dispatch system based logic
		 *                  <> stop
		 *                  | - drop scene / actors space logic
		 *                  | - shutdown user infused system logic
		 *                  | - de-synchronize graphics loop
		 *                  | - de-synchronize audio loop
		 *                  | - de-synchronize physics loop
		 *                  | - de-synchronize input loop
		 *              < shutdown
		 *              | - drop generative tasks
		 *              | - drop loading tasks
		 *              | - drop streaming tasks
		 *              | - release non-system assets
		 *              | - release non-system resources
		 *              | - disable secondary modules
		 *              | - drop unlinked secondary modules
		 *              | - drop left schedules tasks
		 *              | - drop system listeners
		 *          [< shutdown]
		 *          (Fine-granular shutdown is not required due to already resolved dependency chains which is a initialization problem)
		 *      < exit
		 *      | - drop left caches
		 *      | - drop left system resources
		 *      | - drop left secondary modules
		 *      | - release all modules including core modules
		 * > Nil
		 */

	private:
		std::atomic_uint_fast8_t _engineState;

	protected:
		bool setEngineState(core::EngineState state_);

	public:
		[[nodiscard]] core::EngineState getEngineState() const noexcept;

	public:
		/**
		 * Invoked before initialization
		 *
		 * @details Used to create primary modules and resource pointers without order dependencies
		 *
		 * @author Julius
		 * @date 08.12.2022
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool preInit() = 0;

		/**
		 * Initialization Stage
		 *
		 * @details Used to resolve module dependencies and sharing of resource pointers, booting up sub-modules and starting primary functions
		 *
		 * @author Julius
		 * @date 08.12.2022
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool init() = 0;

		/**
		 * Invoked after initialization
		 *
		 * @details Used to dispatch generative, loading or streaming operations based on initialized modules and resources
		 *
		 * @author Julius
		 * @date 08.12.2022
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool postInit() = 0;

		/**
		 * Start running the engine core loops and logic
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool start() = 0;

		/**
		 * Stop running the engine core loops and logic
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool stop() = 0;

		/**
		 * Shutdown operating logic
		 *
		 * @details Will release secondary resources and module, stop generative, loading or streaming logic and shutdown captured logic.
		 *
		 * @author Julius
		 * @date 08.12.2022
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool shutdown() = 0;

		/**
		 * Exit the engine scope
		 *
		 * @details Will release left resources and modules completely
		 *
		 * @author Julius
		 * @date 08.12.2022
		 *
		 * @blocking At least until attempted state change
		 *
		 * @returns True if succeeded, otherwise false due to failure or invalid state change
		 */
		virtual bool exit() = 0;

	public:
		[[nodiscard]] virtual nmpt<Assets> getAssets() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Audio> getAudio() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Graphics> getGraphics() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Input> getInput() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Network> getNetwork() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Physics> getPhysics() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Platform> getPlatform() const noexcept = 0;

		[[nodiscard]] virtual nmpt<ResourceManager> getResources() const noexcept = 0;

		[[nodiscard]] virtual nmpt<Scheduler> getScheduler() const noexcept = 0;

	public:
		[[nodiscard]] virtual ref<Config> getConfig() const noexcept = 0;

		[[nodiscard]] virtual ref<GlobalEventEmitter> getEmitter() const noexcept = 0;

		[[nodiscard]] virtual ref<core::Modules> getModules() const noexcept = 0;

	public:
		[[nodiscard]] virtual Vector<nmpt<core::WorldContext>> getWorldContexts() const noexcept = 0;

	public:
		virtual void addWorld(cref<sptr<core::World>> world_) = 0;

		virtual void removeWorld(cref<sptr<core::World>> world_) = 0;
	};
}
