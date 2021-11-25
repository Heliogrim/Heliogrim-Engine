#pragma once

#include <Engine.ACS/Traits.hpp>

#include "ActorComponent.hpp"
#include "ActorInitializer.hpp"
#include "Inbuilt.hpp"

namespace ember {

    /**
     * Forward Declaration
     */
    class Actor;

    /**
     * Forward Declaration
     */
    struct ActorClass;

    /**
     * Forward Declaration
     */
    struct SerializedActor;

    /**
     * Create a new default actor object
     *
     * @author Julíus
     * @date 25.11.2021
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern future<ptr<Actor>> CreateActor() noexcept;

    /**
     * Creates a new actor object based on given actor class
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The actor class to instantiate
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern future<ptr<Actor>> CreateActor(cref<ActorClass> class_) noexcept;

    /**
     * Will mount the given actor into engine state
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param actor_ The actor to mount into engine state
     *
     * @returns True if succeeded, otherwise false
     */
    [[nodiscard]] extern bool MountActor(const ptr<Actor> actor_) noexcept;

    /**
     * Creates a new actor object equivalent to given actor object
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param actor_ The actor to clone from.
     *
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern future<ptr<Actor>> CloneActor(const ptr<Actor> actor_) noexcept;

    /**
     * Creates a new actor object based on given actor class and mounts it
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param class_ The actor class to instantiate.
     *
     * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern future<ptr<Actor>> SpawnActor(cref<ActorClass> class_) noexcept;

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
     *
     * @returns A future, containing the newly created and mounted actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern future<ptr<Actor>> SpawnActor(cref<ActorClass> class_,
        const ptr<SerializedActor> serialized_) noexcept;

    /**
     * Destroys the given actor
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param actor_ The actor to destroy.
     *
     * @returns A future, representing whether the actor was successfully destroyed.
     */
    [[nodiscard]] extern future<bool> Destroy(mref<ptr<Actor>> actor_) noexcept;

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
         * Constructor
         *
         * @author Julius
         * @date 25.11.2021
         *
         * @param initializer_ The initializer to instantiate subobjects...
         */
        Actor(const ptr<ActorInitializer> initializer_) noexcept;

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

    protected:
        ptr<ActorComponent> _rootComponent;

    public:
        [[nodiscard]] ptr<ActorComponent> getRootComponent() const noexcept;

        [[nodiscard]] cref<math::Transformation> getWorldTransform() const noexcept;
    };
}
