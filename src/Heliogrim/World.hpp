#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Assets/AssetGuid.hpp>

#include "Actor.hpp"
#include "Future.hpp"

namespace hg {
    class Level;
}

namespace hg {
    class World {
    public:
        using this_type = World;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        World();

        /**
         * Constructor
         *
         * @author Julius
         * @date 01.12.2021
         */
        World(const managed<void> internal_);

        World(cref<this_type> other_) = default;

        World(mref<this_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        ~World() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns A shallow copy of this.
         */
        ref<World> operator=(cref<World> other_) = default;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns A shallow copy of this.
         */
        ref<World> operator=(mref<World> other_) noexcept = default;

    public:
        [[nodiscard]] bool valid() const noexcept;

    private:
        /**
         * Internal
         */
        managed<void> _internal;

    public:
        /**
         * Gets the internal state
         *
         * @author Julius
         * @date 04.12.2022
         *
         * @returns A reference to the internal managed world representation
         */
        [[nodiscard]] cref<managed<void>> unwrap() const noexcept;

    public:
        /**
         * Tries to add the given level to this world
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  level_ The level.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool addLevel(_In_ const ptr<Level> level_);

        /**
         * Removes the level from this world
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  level_ The level.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool removeLevel(_In_ const ptr<Level> level_);

    public:
        /**
         * Tries to add the given actor to this world
         *
         * @author Julius
         * @date 01.12.2021
         *
         * @param actor_ The actor to add.
         *
         * @returns True if succeeded, otherwise false.
         */
        bool addActor(const ptr<Actor> actor_);

        /**
         * Removes an actor from this world
         *
         * @author Julius
         * @date 01.12.2021
         *
         * @param actor_ The actor to remove.
         *
         * @returns True if succeeded, otherwise false.
         */
        bool removeActor(const ptr<Actor> actor_);
    };

    /**
     * Creates a new world instance
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @returns A future, containing the newly created world
     */
    [[nodiscard]] extern Future<World> CreateWorld() noexcept;

    /**
     * Query the session's current world
     *
     * @author Julius
     * @date 09.12.2022
     *
     * @param session_ The session where to lookup the current world.
     *
     * @returns A representation for the currently internal stored world.
     */
    [[nodiscard]] extern World GetWorld(cref<Session> session_) noexcept;

    /**
     * Query the current primary core session's world representation
     *
     * @details A convenient shortcut for obtaining the current primary
     *  core session's world instance.
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @returns A representation for the current internal world.
     */
    [[nodiscard]] extern World GetWorld() noexcept;

    /**
     * Gets a world instance by it's guid
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param guid_ The guid of the world to lookup.
     *
     * @returns A future, containing a world instance
     */
    [[nodiscard]] extern Future<World> GetWorld(cref<asset_guid> guid_) noexcept;

    /**
     * Destroys the given world and it's implicit resources
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param world_ The world instance to destroy.
     *
     * @returns A future, representing whether the world was successfully destroyed.
     */
    extern Future<bool> Destroy(mref<World> world_);

    /* Warning: Experimental Feature */
    /**
     * Set's the world of a core session
     *
     * @note The world *must* have been created with a engine wide propagation,
     *  otherwise this would result in ub.
     *
     * @warning This method is currently unsafe. The implicit internal changes
     *  aspecially at the session's world-context may result in memory corruption or
     *  modify-on-read behaviour. Further this change is required to propagate through
     *  whole engine and modules, therefore providing an endless amount of conflict points.
     *
     * @note We might want this to return a future result, due to the most likely asynchronous fix
     *  to the described problems. This might result in a full schedule tick to complete the
     *  required changes.
     *
     * @author Julius
     * @date 10.12.2022
     *
     * @param session_ The session where to set the current world
     * @param world_ The world to use next.
     */
    extern void SetWorld(cref<Session> session_, cref<World> world_);

    /* Warning: Experimental Feature */
    /**
     * Transition from the session's current world to another one
     *
     * @author Julius
     * @date 10.12.2022
     *
     * @see ::hg::SetWorld(...)
     */
    extern void TransitionToWorld(cref<Session> session_, cref<World> world_);
}
