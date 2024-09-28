#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "LightComponent.hpp"

namespace hg {
	class SpotLightComponent :
		public InheritMeta<SpotLightComponent, LightComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = SpotLightComponent;

		constexpr static component_type_id typeId { "SpotLightComponent"_typeId };

	public:
		SpotLightComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~SpotLightComponent() override;

	private:
	public:
		f32 _innerCone;
		f32 _outerCone;
		f32 _distanceLimit;
		math::fvec3 _luminance;
	};
}
