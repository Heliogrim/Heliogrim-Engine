#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/UniqueRef.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace JPH {
	class PhysicsSystem;
}

namespace hg::engine::physics {
	class JphPhysicsSystemHooks;
}

namespace hg::engine {
	class Physics :
		public core::CoreModule {
	public:
		Physics(ref<Engine> engine_) noexcept;

		~Physics() override;

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
