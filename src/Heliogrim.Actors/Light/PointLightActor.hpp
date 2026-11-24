#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Graphics.Components;
#endif

namespace hg {
	class PointLightActor :
		public InheritArchetype<PointLightActor, Actor> {
	public:
		using this_type = PointLightActor;
		constexpr static type_id typeId { "PointLightActor"_typeId };

		using layout = engine::acs::ActorLayout<
			engine::acs::Slot<"point", PointLightComponent>>;

	public:
		explicit PointLightActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<PointLightComponent> getPointLightComponent() noexcept;
	};
}
