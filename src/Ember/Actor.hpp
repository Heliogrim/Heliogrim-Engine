#pragma once

#include <Engine.ACS/Traits.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Math/Transform.hpp>
#include <Engine.Reflect/EmberReflect.hpp>

#if TRUE
#include <Engine.Reflect/Reflect.hpp>
#endif

#include "Future.hpp"
#include "Traits.hpp"

namespace ember {
    /**
     * Forward Declaration
     */
    struct ActorClass;
    class ActorComponent;
    class ActorInitializer;
    struct SerializedActor;

    class IComponentRegisterContext;
    class Session;
    class World;

    /**
     * The base class of every actor
     *
     * @author Julius
     * @date 25.11.2021
     */
    class Actor :
        public EmberObject {
    public:
        using this_type = Actor;
        using underlying_type = EmberObject;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 25.11.2021
         */
        Actor() noexcept;

        // Warning: Temporary Debug Solution
        Actor(mref<Actor> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.11.2021
         */
        ~Actor() noexcept;

    public:
        /**
         * Move Assignment
         *
         * @author Julius
         * @date 25.11.2021
         */
        ref<Actor> operator=(mref<Actor>) noexcept = delete;

        /**
         * Copy Assignment
         *
         * @author Julius
         * @date 25.11.2021
         */
        ref<Actor> operator=(cref<Actor>) = delete;

    private:
        actor_guid _guid;

    public:
        /**
         * Gets the unique identifier of this actor object
         *
         * @author Julius
         * @date 25.11.2021
         *
         * @returns The actor_guid of this actor object.
         */
        [[nodiscard]] actor_guid guid() const noexcept;

        void unsafe_set_guid(cref<actor_guid> guid_);

    protected:
        ptr<ActorComponent> _rootComponent;

    public:
        [[nodiscard]] ptr<ActorComponent> getRootComponent() const noexcept;

        [[nodiscard]] cref<math::Transform> getWorldTransform() const noexcept;

    private:
        CompactSet<ptr<ActorComponent>> _components;

    private:
        template <typename Fnc_>
        void eachComponent(mref<Fnc_> fnc_) const {
            /**
             *
             */
            for (auto& entry : _components) {
                fnc_(entry);
            }
        }

    public:
        [[nodiscard]] cref<CompactSet<ptr<ActorComponent>>> getComponents() const noexcept;

        template <typename Selector_>
        [[nodiscard]] CompactArray<ptr<ActorComponent>> selectComponents(mref<Selector_> selector_) const {
            CompactArray<ptr<ActorComponent>> result {};
            eachComponent([&](const ptr<ActorComponent> component_) {
                if (selector_(component_)) {
                    result.push_back(component_);
                }
            });

            return result;
        }

        void addComponent(const ptr<ActorComponent> component_);

    public:
        /**
         * Callback to register all related components of this actor
         *
         * @author Julius
         * @date 01.12.2021
         *
         * @param context_ The context where to register components.
         */
        void registerComponents(const ptr<IComponentRegisterContext> context_);
    };

    /**
     * Registers a new ActorClass to the engine
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The reflected actor class instance to register.
     *
     * @returns True if register succeeded, otherwise false
     */
    extern bool RegisterActorClass(cref<ActorClass> class_) noexcept;

    /**
     * Registers a new ActorType to the engine
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @details This template invocation will automatically generate a reflected ActorClass to register to the engine
     *
     * @tparam ActorType_ The actor class type to register
     *
     * @returns True if register succeeded, otherwise false
     */
    template <class ActorType_>
    bool RegisterActorClass() noexcept {
        return RegisterActorClass(Reflect::SubstitudeActorClass<ActorType_> {});
    }

    /**
     * Create a new default actor object
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param session_ The core session where to create the actor
     *
     * @returns A pointer to the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern ptr<Actor> CreateActor(cref<Session> session_);

    /**
     * Create a new default actor object
     *
     * @details An active or mounted world is guaranteed to be scoped with a session,
     *  therefore a data management layer is resolvable. This will not populate the scene
     *  like calling a actor spawn function.
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param activeWorld_ The world where to create the actor
     *
     * @returns A pointer to the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern ptr<Actor> CreateActor(cref<World> activeWorld_);

    /**
     * Create a new default actor object
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param session_ The core session where to create the actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(cref<Session> session_, async_t);

    /**
     * Create a new default actor object
     *
     * @details An active or mounted world is guaranteed to be scoped with a session,
     *  therefore a data management layer is resolvable. This will not populate the scene
     *  like calling a actor spawn function.
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param activeWorld_ The world where to create the actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(cref<World> activeWorld_, async_t);

    /**
     * Creates a new actor object based on given actor class
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The actor class to instantiate
     * @param session_ The core session where to create the actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(
        cref<ActorClass> class_,
        cref<Session> session_
    ) noexcept;

    /**
     * Creates a new actor object based on given actor class
     *
     * @details An active or mounted world is guaranteed to be scoped with a session,
     *  therefore a data management layer is resolvable. This will not populate the scene
     *  like calling a actor spawn function.
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The actor class to instantiate
     * @param activeWorld_ The world where to create the actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(
        cref<ActorClass> class_,
        cref<World> activeWorld_
    ) noexcept;

    /**
     * Create a new actor object based on given actor class
     *
     * @author Julius
     * @date 26.11.2021
     *
     * @param class_ The actor class to instantiate.
     * @param serialized_ The serialized data to use.
     * @param session_ The core session where to create the actor
     *
     * @returns A future, containing the newly created actor is succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(
        cref<ActorClass> class_,
        const ptr<SerializedActor> serialized_,
        cref<Session> session_
    ) noexcept;

    /**
     * Create a new actor object based on given actor class
     *
     * @details An active or mounted world is guaranteed to be scoped with a session,
     *  therefore a data management layer is resolvable. This will not populate the scene
     *  like calling a actor spawn function.
     *
     * @author Julius
     * @date 26.11.2021
     *
     * @param class_ The actor class to instantiate.
     * @param serialized_ The serialized data to use.
     * @param activeWorld_ The world where to create the actor
     *
     * @returns A future, containing the newly created actor is succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(
        cref<ActorClass> class_,
        const ptr<SerializedActor> serialized_,
        cref<World> activeWorld_
    ) noexcept;

    /**
     * Creates a new actor object equivalent to given actor object
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param actor_ The actor to clone from.
     * @param session_ The core session where to create the new actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CloneActorInto(
        const ptr<Actor> actor_, cref<Session> session_
    ) noexcept;

    /**
     * Creates a new actor object equivalent to given actor object
     *
     * @details An active or mounted world is guaranteed to be scoped with a session,
     *  therefore a data management layer is resolvable. This will not populate the scene
     *  like calling a actor spawn function.
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param actor_ The actor to clone from.
     * @param activeWorld_ The world where to create the new actor
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CloneActorInto(
        const ptr<Actor> actor_, cref<World> activeWorld_
    ) noexcept;

    /**
     * Creates a new actor object based on given actor class and mounts it
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The actor class to instantiate.
     * @param activeWorld_ The world where to create and place the new actor
     *
     * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> SpawnActor(
        cref<ActorClass> class_, cref<World> activeWorld_
    ) noexcept;

    /**
     * Creates a new actor object based on given actor class and mounts it
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @details Will populate the instantiated actor by given serialized data
     *
     * @param class_ The actor class to instantiate.
     * @param serialized_ The serialized data to use.
     * @param activeWorld_ The world where to create and place the new actor
     *
     * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> SpawnActor(
        cref<ActorClass> class_,
        const ptr<SerializedActor> serialized_,
        cref<World> activeWorld_
    ) noexcept;

    /**
     * Destroys the given actor
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param session_ The session where to destroy the actor.
     * @param actor_ The actor to destroy.
     *
     * @returns A future, representing whether the actor was successfully destroyed.
     */
    [[nodiscard]] extern Future<bool> Destroy(cref<Session> session_, mref<ptr<Actor>> actor_) noexcept;

    /**
     * Destroys the given actor
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param activeWorld_ The world where to destroy the actor.
     * @param actor_ The actor to destroy.
     *
     * @returns A future, representing whether the actor was successfully destroyed.
     */
    [[nodiscard]] extern Future<bool> Destroy(cref<World> activeWorld_, mref<ptr<Actor>> actor_) noexcept;
}
