#pragma once

#include <Engine.Graphics.Components/Geometry/StaticGeometryComponent.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

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
