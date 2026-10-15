#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "ShapeCollisionComponent.hpp"

namespace hg {
	class BoxShapeCollisionComponent final :
		public InheritMeta<BoxShapeCollisionComponent, ShapeCollisionComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = BoxShapeCollisionComponent;
		using underlying_type = ShapeCollisionComponent;

		constexpr static ComponentTypeId typeId { "BoxShapeCollisionComponent"_typeId };

	public:
		BoxShapeCollisionComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~BoxShapeCollisionComponent() override = default;

	private:
	public:
		struct Settings {
			bool isStatic : 1;
			bool isTrigger : 1;
		} settings;

		math::Vector3f32 extent;
	};
}
