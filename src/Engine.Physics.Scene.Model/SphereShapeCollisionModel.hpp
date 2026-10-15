#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.PFX/jolt.inc.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystemModel.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

namespace hg::engine::physics {
	class SphereShapeCollisionModel final :
		public InheritMeta<SphereShapeCollisionModel, PhysicsSceneSystemModel> {
	public:
		using this_type = SphereShapeCollisionModel;

	public:
		SphereShapeCollisionModel(ref<SceneComponent> owner_);

		SphereShapeCollisionModel(mref<this_type>) noexcept = default;

		~SphereShapeCollisionModel() override;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

	public:
		void create(ref<PhysicsSceneSystem> system_) override;

		void update(ref<PhysicsSceneSystem> system_) override;

		void readback(ref<PhysicsSceneSystem> system_) override;

		void destroy(ref<PhysicsSceneSystem> system_) override;

	private:
		JPH::SphereShapeSettings _sphereShapeSettings;
		JPH::Ref<JPH::Shape> _sphereShape;
		Opt<ref<JPH::Body>> _body;
	};
}
