#pragma once

#include <Engine.Graphics.Components/Light/DirectionalLightComponent.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

namespace hg {
	class DirectionalLightActor :
		public InheritMeta<DirectionalLightActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = DirectionalLightActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "DirectionalLightActor"_typeId };

	public:
		DirectionalLightActor(cref<ActorInitializer> initializer_);

	private:
		ptr<DirectionalLightComponent> _directionalLightComponent;

	public:
		[[nodiscard]] nmpt<DirectionalLightComponent> getDirectionalLightComponent() const noexcept;
	};
}
