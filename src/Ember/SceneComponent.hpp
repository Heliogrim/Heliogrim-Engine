#pragma once
#include "ActorComponent.hpp"

namespace ember {

    class SceneComponent :
        public ActorComponent {
    public:
        using this_type = SceneComponent;
        using underlying_type = ActorComponent;

    public:
        SceneComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);
    };
}
