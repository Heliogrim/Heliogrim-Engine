#pragma once
#include <Engine.Common/Collection/Vector.hpp>

#include "ActorComponent.hpp"

namespace hg {
    struct SceneNodeModel;
}

namespace hg {
    class SceneComponent :
        public InheritMeta<SceneComponent, ActorComponent> {
    public:
        using this_type = SceneComponent;
        using underlying_type = ActorComponent;

    public:
        SceneComponent(
            mref<component_type_id> typeId_,
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );

        ~SceneComponent();
    };
}
