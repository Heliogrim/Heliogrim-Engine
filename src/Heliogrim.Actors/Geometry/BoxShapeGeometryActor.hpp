#pragma once

#include <Engine.Graphics.Components/Geometry/BoxShapeGeometryComponent.hpp>
#include <Engine.Physics.Components/Collision/BoxShapeCollisionComponent.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

namespace hg {
	class BoxShapeGeometryActor :
		public InheritArchetype<BoxShapeGeometryActor, Actor> {
	public:
		using this_type = BoxShapeGeometryActor;
		constexpr static type_id typeId { "BoxShapeGeometryActor"_typeId };

		using layout = engine::acs::ActorLayout<
			engine::acs::Slot<"collision", BoxShapeCollisionComponent, engine::acs::Children<
				engine::acs::Slot<"geometry", BoxShapeGeometryComponent>>>>;

	public:
		explicit BoxShapeGeometryActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<BoxShapeCollisionComponent> getBoxShapeCollisionComponent() noexcept;

		[[nodiscard]] nmpt<BoxShapeGeometryComponent> getBoxShapeGeometryComponent() noexcept;
	};
}
