#pragma once
#include "SceneComponent.hpp"

namespace hg {
    class LightComponent :
        public InheritMeta<LightComponent, SceneComponent> {
    public:
        using this_type = LightComponent;
        using underlying_type = SceneComponent;

    public:
        LightComponent(
            mref<component_type_id> typeId_,
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );
    };
}
