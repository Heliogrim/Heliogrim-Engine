#pragma once
#include "SceneComponent.hpp"

namespace ember {

    class UIComponent :
        public SceneComponent {
    public:
        using this_type = UIComponent;
        using underlying_type = SceneComponent;

        inline constexpr static component_type_id typeId { "UIComponent"_typeId };

    public:
        UIComponent(
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );

        ~UIComponent() = default;
    };
}
