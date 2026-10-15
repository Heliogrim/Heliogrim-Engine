#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Collision/SphereShapeCollisionComponent.hpp"
#include "../../Component/Scene/Geometry/SphereShapeGeometryComponent.hpp"

namespace hg {
	class SphereShapeGeometryActor :
		public InheritMeta<SphereShapeGeometryActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SphereShapeGeometryActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "SphereShapeGeometryActor"_typeId };

	public:
		SphereShapeGeometryActor(cref<ActorInitializer> initializer_);

	private:
		ptr<SphereShapeCollisionComponent> _sphereShapeCollisionComponent;
		ptr<SphereShapeGeometryComponent> _sphereShapeGeometryComponent;

	public:
		[[nodiscard]] nmpt<SphereShapeCollisionComponent> getSphereShapeCollisionComponent() const noexcept;

		[[nodiscard]] nmpt<SphereShapeGeometryComponent> getSphereShapeGeometryComponent() const noexcept;
	};
}
