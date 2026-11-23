#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "LightComponent.hpp"

namespace hg {
	class PointLightComponent :
		public InheritMeta<PointLightComponent, LightComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = PointLightComponent;

		constexpr static ComponentTypeId typeId { "PointLightComponent"_typeId };

	public:
		PointLightComponent(mref<ComponentGuid> guid_, mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~PointLightComponent() override;

	private:
	public:
		f32 _distanceLimit;
		math::fvec3 _luminance;
	};
}