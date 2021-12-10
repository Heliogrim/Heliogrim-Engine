#pragma once

#include <Engine.ACS/Traits.hpp>

#include "ActorComponent.hpp"
#include "Inbuilt.hpp"

namespace ember {

    /**
     * Forward Declaration
     */
    struct ActorClass;
    class IComponentRegisterContext;
    struct SerializedActor;

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
        [[nodiscard]] Vector<ptr<ActorComponent>> selectComponents(mref<Selector_> selector_) const {

            Vector<ptr<ActorComponent>> result {};
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
     * @returns A future, containing the newly created actor if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor() noexcept;

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
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(cref<ActorClass> class_) noexcept;

    /**
     * Create a new actor object based on given actor class
     *
     * @author Julius
     * @date 26.11.2021
     *
     * @param class_ The actor class to instantiate.
     * @param serialized_ The serialized data to use.
     */
    [[nodiscard]] extern Future<ptr<Actor>> CreateActor(cref<ActorClass> class_,
        const ptr<SerializedActor> serialized_) noexcept;

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
    [[nodiscard]] extern Future<ptr<Actor>> CloneActor(const ptr<Actor> actor_) noexcept;

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
    [[nodiscard]] extern Future<ptr<Actor>> SpawnActor(cref<ActorClass> class_) noexcept;

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
    [[nodiscard]] extern Future<ptr<Actor>> SpawnActor(cref<ActorClass> class_,
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
    [[nodiscard]] extern Future<bool> Destroy(mref<ptr<Actor>> actor_) noexcept;
}
