#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "LightComponent.hpp"

namespace hg {
	class DirectionalLightComponent :
		public InheritMeta<DirectionalLightComponent, LightComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = DirectionalLightComponent;

		constexpr static ComponentTypeId typeId { "DirectionalLightComponent"_typeId };

	public:
		DirectionalLightComponent(mref<ComponentGuid> guid_, mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~DirectionalLightComponent() override;

	private:
	public:
		math::fvec3 _luminance;
		math::Rotator _direction;
	};
}