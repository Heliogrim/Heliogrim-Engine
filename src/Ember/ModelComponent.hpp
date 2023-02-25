#pragma once
#include "SceneComponent.hpp"

namespace ember {
    class ModelComponent :
        public SceneComponent {
    public:
        using this_type = ModelComponent;
        using underlying_type = SceneComponent;

    public:
        ModelComponent(
            mref<component_type_id> typeId_,
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );

        ~ModelComponent() = default;
    };
}
