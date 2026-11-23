#pragma once

#include <Engine.ACS/Actor/Actor.hpp>
#include <Engine.ACS/Archetype/InheritArchetype.hpp>
#include <Engine.ACS/Archetype/Layout.hpp>

#include <Engine.Physics.Components/Collision/BoxShapeCollisionComponent.hpp>
#include <Engine.Graphics.Components/Geometry/BoxShapeGeometryComponent.hpp>

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
