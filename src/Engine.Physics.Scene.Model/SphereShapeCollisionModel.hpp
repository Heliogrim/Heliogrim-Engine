#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <Engine.Common/__macro.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystemModel.hpp>
#include <Engine.Physics/jolt.inc.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
