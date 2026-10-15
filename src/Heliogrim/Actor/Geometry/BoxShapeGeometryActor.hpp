#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Collision/BoxShapeCollisionComponent.hpp"
#include "../../Component/Scene/Geometry/BoxShapeGeometryComponent.hpp"

namespace hg {
	class BoxShapeGeometryActor :
		public InheritMeta<BoxShapeGeometryActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = BoxShapeGeometryActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "BoxShapeGeometryActor"_typeId };

	public:
		BoxShapeGeometryActor(cref<ActorInitializer> initializer_);

	private:
		ptr<BoxShapeCollisionComponent> _boxShapeCollisionComponent;
		ptr<BoxShapeGeometryComponent> _boxShapeGeometryComponent;

	public:
		[[nodiscard]] nmpt<BoxShapeCollisionComponent> getBoxShapeCollisionComponent() const noexcept;

		[[nodiscard]] nmpt<BoxShapeGeometryComponent> getBoxShapeGeometryComponent() const noexcept;
	};
}
