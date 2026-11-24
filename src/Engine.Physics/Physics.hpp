#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace JPH {
	class PhysicsSystem;
}

namespace hg::engine::physics {
	class JphPhysicsSystemHooks;
}

namespace hg::engine {
	class Physics :
		public core::CoreModule {
	private:
		friend class ::hg::engine::core::ModuleSlotAccess;

	public:
		Physics(ref<Engine> engine_) noexcept;

		~Physics() override;

	private:
		static constinit std::atomic<ptr<Physics>> _moduleSlot;

	public:
		[[nodiscard]] static Opt<ref<Physics>> get() noexcept {
			return engine::module<Physics>();
		}

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		Vector<std::pair<UniquePtr<JPH::PhysicsSystem>, UniquePtr<physics::JphPhysicsSystemHooks>>> _systems;

	public:
		[[nodiscard]] UniqueRef<JPH::PhysicsSystem> acquireSystem();

		void releaseSystem(mref<UniqueRef<JPH::PhysicsSystem>> system_);
	};
}
