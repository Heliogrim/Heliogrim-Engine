#pragma once

#include <Engine.Session/Session.hpp>

#include "Actor.hpp"
#include "Inbuilt.hpp"

namespace ember {

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
        static ref<ActorInitializer> get() noexcept;

    public:
        template <std::derived_from<ActorComponent> Component>
        ptr<Component> createComponent(const ptr<Actor> actor_) {

            /**
             *
             */
            auto session { engine::Session::get() };

            auto* component { new Component({ actor_->guid(), actor_ }, nullptr) };

            /**
             *
             */
            assert(actor_->getRootComponent() == nullptr &&
                "Failed to ensure no existing root component at referenced actor.");
            actor_->addComponent(component);

            return component;
        }

        template <std::derived_from<ActorComponent> Component>
        ptr<Component> createSubComponent(const ptr<Actor> actor_, const ptr<ActorComponent> parent_) {

            /**
             *
             */
            auto session { engine::Session::get() };
            auto* actor { actor_ ? actor_ : parent_->getOwner() };

            auto* component { new Component({ actor_->guid(), actor }, parent_) };

            /**
             *
             */
            // assert(actor->hasComponent(parent_), "Failed to ensure existing parent component at referenced actor.");
            actor_->addComponent(component);

            return component;
        }
    };

}
