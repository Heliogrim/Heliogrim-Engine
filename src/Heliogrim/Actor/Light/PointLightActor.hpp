#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Light/PointLightComponent.hpp"

namespace hg {
	class PointLightActor :
		public InheritMeta<PointLightActor, Actor> {
	public:
		using this_type = PointLightActor;
		using underlying_type = Actor;

	public:
		PointLightActor(cref<ActorInitializer> initializer_);

	private:
		ptr<PointLightComponent> _pointLightComponent;

	public:
		[[nodiscard]] nmpt<PointLightComponent> getPointLightComponent() const noexcept;
	};
}
