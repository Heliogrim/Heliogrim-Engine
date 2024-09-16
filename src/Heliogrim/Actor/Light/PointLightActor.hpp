#pragma once

#include "../Actor.hpp"
#include "../../Component/Scene/Light/PointLightComponent.hpp"

namespace hg {
	class PointLightActor :
		public InheritMeta<PointLightActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = PointLightActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "PointLightActor"_typeId };

	public:
		PointLightActor(cref<ActorInitializer> initializer_);

	private:
		ptr<PointLightComponent> _pointLightComponent;

	public:
		[[nodiscard]] nmpt<PointLightComponent> getPointLightComponent() const noexcept;
	};
}
