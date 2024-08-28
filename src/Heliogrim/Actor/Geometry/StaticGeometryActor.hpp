#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Geometry/StaticGeometryComponent.hpp"

namespace hg {
	class StaticGeometryActor :
		public InheritMeta<StaticGeometryActor, Actor> {
	public:
		using this_type = StaticGeometryActor;
		using underlying_type = Actor;

	public:
		StaticGeometryActor(cref<ActorInitializer> initializer_);

	private:
		ptr<StaticGeometryComponent> _staticGeometryComponent;

	public:
		[[nodiscard]] nmpt<StaticGeometryComponent> getStaticGeometryComponent() const noexcept;
	};
}
