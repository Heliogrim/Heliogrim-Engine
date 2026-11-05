#pragma once

#include <Engine.ACS/Archetype/Layout.hpp>

#include "../Actor.hpp"
#include "../../Component/Scene/Geometry/StaticGeometryComponent.hpp"
#include "../Archetype/InheritArchetype.hpp"

namespace hg {
	class StaticGeometryActor :
		public InheritArchetype<StaticGeometryActor, Actor> {
	public:
		using this_type = StaticGeometryActor;
		constexpr static type_id typeId { "StaticGeometryActor"_typeId };

		using layout = engine::acs::ActorLayout<engine::acs::Slot<"geometry", StaticGeometryComponent>>;

	public:
		explicit StaticGeometryActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<StaticGeometryComponent> getStaticGeometryComponent() noexcept;
	};
}
