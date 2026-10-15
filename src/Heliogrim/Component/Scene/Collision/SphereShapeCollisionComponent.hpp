#pragma once
#include "ShapeCollisionComponent.hpp"
#include "Engine.Reflect/Inherit/InheritMeta.hpp"

namespace hg {
	class SphereShapeCollisionComponent final :
		public InheritMeta<SphereShapeCollisionComponent, ShapeCollisionComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SphereShapeCollisionComponent;
		using underlying_type = ShapeCollisionComponent;

		constexpr static ComponentTypeId typeId { "SphereShapeCollisionComponent"_typeId };

	public:
		SphereShapeCollisionComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~SphereShapeCollisionComponent() override = default;

	private:
	public:
		struct Settings {
			bool isStatic : 1;
			bool isTrigger : 1;
		} settings;

		math::Vector1f32 radius;
	};
}
