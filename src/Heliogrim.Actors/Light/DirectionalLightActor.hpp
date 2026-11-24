#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Graphics.Components;
#endif

namespace hg {
	class DirectionalLightActor :
		public InheritArchetype<DirectionalLightActor, Actor> {
	public:
		using this_type = DirectionalLightActor;
		constexpr static type_id typeId { "DirectionalLightActor"_typeId };

		using layout = engine::acs::ActorLayout<
			engine::acs::Slot<"directional", DirectionalLightComponent>>;

	public:
		explicit DirectionalLightActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<DirectionalLightComponent> getDirectionalLightComponent() noexcept;
	};
}
