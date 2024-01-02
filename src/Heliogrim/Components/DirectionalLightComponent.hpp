#pragma once

#include "../LightComponent.hpp"

namespace hg {
    class DirectionalLightComponent :
        public InheritMeta<DirectionalLightComponent, LightComponent> {
    public:
        using this_type = DirectionalLightComponent;

        constexpr static component_type_id typeId { "DirectionalLightComponent"_typeId };

    public:
        DirectionalLightComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

        ~DirectionalLightComponent();

    private:
    public:
        math::fvec3 _luminance;
        math::Rotator _direction;
    };
}
