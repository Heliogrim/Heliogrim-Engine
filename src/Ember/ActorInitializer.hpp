#pragma once

#include "Inbuilt.hpp"

namespace ember {

    /**
     * Forward Declaration
     */
    class Actor;

    /**
     * Forward Declaration
     */
    class ActorComponent;

    class ActorInitializer {
    public:
        using this_type = ActorInitializer;

    protected:
        ActorInitializer() = default;

        ~ActorInitializer() noexcept = default;

    public:
        template <std::derived_from<ActorComponent> Component>
        ptr<Component> createComponent(ptr<Actor> actor_) {
            return nullptr;
        }

        template <std::derived_from<ActorComponent> Component>
        ptr<Component> createSubComponent(ptr<ActorComponent> parent_) {
            return nullptr;
        }
    };

}
