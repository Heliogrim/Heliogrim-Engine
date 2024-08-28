#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Light/DirectionalLightComponent.hpp"

namespace hg {
	class DirectionalLightActor :
		public InheritMeta<DirectionalLightActor, Actor> {
	public:
		using this_type = DirectionalLightActor;
		using underlying_type = Actor;

	public:
		DirectionalLightActor(cref<ActorInitializer> initializer_);

	private:
		ptr<DirectionalLightComponent> _directionalLightComponent;

	public:
		[[nodiscard]] nmpt<DirectionalLightComponent> getDirectionalLightComponent() const noexcept;
	};
}
