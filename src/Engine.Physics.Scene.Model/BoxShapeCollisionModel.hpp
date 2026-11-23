#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>

/**/

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Physics;
import Engine.Physics.Scene;
import Engine.Reflect;
#endif

namespace hg::engine::physics {
	class BoxShapeCollisionModel final :
		public InheritMeta<BoxShapeCollisionModel, PhysicsSceneSystemModel> {
	public:
		using this_type = BoxShapeCollisionModel;

	public:
		BoxShapeCollisionModel(ref<SceneComponent> owner_);

		BoxShapeCollisionModel(mref<this_type>) noexcept = default;

		~BoxShapeCollisionModel() override;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

	public:
		void create(ref<PhysicsSceneSystem> system_) override;

		void update(ref<PhysicsSceneSystem> system_) override;

		void readback(ref<PhysicsSceneSystem> system_) override;

		void destroy(ref<PhysicsSceneSystem> system_) override;

	private:
		JPH::BoxShapeSettings _boxShapeSettings;
		JPH::Ref<JPH::Shape> _boxShape;
		Opt<ref<JPH::Body>> _body;
	};
}
