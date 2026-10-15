#pragma once

#include <Engine.Common/Managed/UniqueRef.hpp>
#include <Engine.Scene/SceneSystem.hpp>

#include "PhysicsSceneRegistry.hpp"

namespace JPH {
	class PhysicsSystem;
	class BodyInterface;
}

namespace hg::engine::physics {
	class PhysicsSceneSystem :
		public scene::SceneSystem<PhysicsSceneSystem> {
	public:
		using this_type = PhysicsSceneSystem;

	public:
		PhysicsSceneSystem();

		~PhysicsSceneSystem() override;

	private:
		PhysicsSceneRegistry _registry;
		UniqueRef<JPH::PhysicsSystem> _physicsSystem;
		Opt<ref<JPH::BodyInterface>> _bodyInterface;

	public:
		[[nodiscard]] ref<PhysicsSceneRegistry> getRegistry() noexcept;

		[[nodiscard]] Opt<ref<JPH::PhysicsSystem>> getPhysicsSystem() noexcept;

		[[nodiscard]] Opt<ref<JPH::BodyInterface>> getJoltBodyInterface() noexcept;

	public:
		void prepare() override;

		void cleanup() override;

	public:
		void broadcast(scene::SceneBroadcastFlags flags_) override;

		void update(scene::SceneUpdateFlags flags_) override;

		void readback();

		void postprocess(scene::ScenePostProcessFlags flags_) override;

	public:
		void add(const ptr<const MetaClass> meta_, std::span<const ptr<SceneComponent>> batch_) override;

		void remove(const ptr<const MetaClass> meta_, std::span<const ptr<const SceneComponent>> batch_) override;

		void clear() override;
	};
}
