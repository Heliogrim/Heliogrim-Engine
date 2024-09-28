#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Geometry/StaticGeometryComponent.hpp"

namespace hg {
	class StaticGeometryActor :
		public InheritMeta<StaticGeometryActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = StaticGeometryActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "StaticGeometryActor"_typeId };

	public:
		StaticGeometryActor(cref<ActorInitializer> initializer_);

	private:
		ptr<StaticGeometryComponent> _staticGeometryComponent;

	public:
		[[nodiscard]] nmpt<StaticGeometryComponent> getStaticGeometryComponent() const noexcept;
	};
}
