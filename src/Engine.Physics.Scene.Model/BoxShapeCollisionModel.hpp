#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.PFX/jolt.inc.hpp>
#include <Engine.Physics.Scene/PhysicsSceneSystemModel.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

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
