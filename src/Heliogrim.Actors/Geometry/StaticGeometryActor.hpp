#pragma once

#include <Engine.ACS/Actor/Actor.hpp>
#include <Engine.ACS/Archetype/InheritArchetype.hpp>
#include <Engine.ACS/Archetype/Layout.hpp>

#include "../../Component/Scene/Geometry/StaticGeometryComponent.hpp"

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
