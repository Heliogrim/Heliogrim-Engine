#pragma once

#include <Engine.ACS/Archetype/Layout.hpp>

#include "../Actor.hpp"
#include "../Archetype/InheritArchetype.hpp"
#include "../../Component/Scene/Collision/SphereShapeCollisionComponent.hpp"
#include "../../Component/Scene/Geometry/SphereShapeGeometryComponent.hpp"

namespace hg {
	class SphereShapeGeometryActor :
		public InheritArchetype<SphereShapeGeometryActor, Actor> {
	public:
		using this_type = SphereShapeGeometryActor;
		constexpr static type_id typeId { "SphereShapeGeometryActor"_typeId };

		using layout = engine::acs::ActorLayout<
			engine::acs::Slot<"collision", SphereShapeCollisionComponent, engine::acs::Children<
				engine::acs::Slot<"geometry", SphereShapeGeometryComponent>>>>;

	public:
		explicit SphereShapeGeometryActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<SphereShapeCollisionComponent> getSphereShapeCollisionComponent() noexcept;

		[[nodiscard]] nmpt<SphereShapeGeometryComponent> getSphereShapeGeometryComponent() noexcept;
	};
}
