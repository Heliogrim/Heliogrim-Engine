#pragma once

#include "../LightComponent.hpp"

namespace hg {
    class SpotLightComponent :
        public InheritMeta<SpotLightComponent, LightComponent> {
    public:
        using this_type = SpotLightComponent;

        constexpr static component_type_id typeId { "SpotLightComponent"_typeId };

    public:
        SpotLightComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

        ~SpotLightComponent();

    private:
    public:
        f32 _innerCone;
        f32 _outerCone;
        f32 _distanceLimit;
        math::fvec3 _luminance;
    };
}
