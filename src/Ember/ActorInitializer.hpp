#pragma once

#include <Engine.Session/Session.hpp>

#include "Actor.hpp"
#include "Inbuilt.hpp"
#include "Engine.ACS/Registry.hpp"

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
        ptr<Component> createComponent(_Inout_ const ptr<Actor> actor_) {

            /**
             *
             */
            const auto& session { engine::Session::get() };
            auto* registry { session->modules().acsRegistry() };

            auto* component {
                registry->acquireActorComponent<
                    Component,
                    CachedActorPointer,
                    ptr<ActorComponent>
                >(actor_->guid(), { actor_->guid(), actor_ }, nullptr)
            };
            assert(component != nullptr && "Failed to ensure successful created component.");

            /**
             *
             */
            assert(actor_->getRootComponent() == nullptr &&
                "Failed to ensure no existing root component at referenced actor.");
            actor_->addComponent(component);

            return component;
        }

        template <std::derived_from<ActorComponent> Component>
        ptr<Component> createSubComponent(_Inout_ const ptr<Actor> actor_, ptr<ActorComponent> parent_) {

            /**
             *
             */
            const auto& session { engine::Session::get() };
            auto* registry { session->modules().acsRegistry() };

            auto* actor { actor_ ? actor_ : parent_->getOwner() };

            auto* component {
                registry->acquireActorComponent<
                    Component,
                    CachedActorPointer,
                    ptr<ActorComponent>
                >(actor->guid(), { actor_->guid(), actor },
                    _STD move(ptr<ActorComponent> { parent_ }))
            };
            assert(component != nullptr && "Failed to ensure successful created component.");

            /**
             *
             */
            // assert(actor->hasComponent(parent_), "Failed to ensure existing parent component at referenced actor.");
            actor_->addComponent(component);

            return component;
        }
    };

}
