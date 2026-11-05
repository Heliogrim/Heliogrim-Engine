#pragma once

#include <Engine.ACS/Archetype/Layout.hpp>

#include "../Actor.hpp"
#include "../../Component/Scene/Collision/BoxShapeCollisionComponent.hpp"
#include "../../Component/Scene/Geometry/BoxShapeGeometryComponent.hpp"
#include "../Archetype/InheritArchetype.hpp"

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
