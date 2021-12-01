#pragma once

#include "Inbuilt.hpp"

#include "Level.hpp"
#include "Scene.hpp"

namespace ember {

    class World {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        World() = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 01.12.2021
         */
        World(const non_owning_rptr<Scene> scene_);

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        World(cref<World>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        World(mref<World>) noexcept = delete;

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
        ref<World> operator=(cref<World>) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns A shallow copy of this.
         */
        ref<World> operator=(mref<World>) noexcept = delete;

    private:
        non_owning_rptr<Scene> _scene;

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
     * @returns A future, containing the newly created world if succeeded, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<World>> CreateWorld() noexcept;

    /**
     * Gets the current active World
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @returns A const pointer to the current active world instance.
     */
    [[nodiscard]] extern const ptr<World> GetWorld() noexcept;

    /**
     * Gets a world instance it's guid
     *
     * @author Julius
     * @date 25.11.2021
     *
     * @param guid_ The guid of the world to lookup.
     *
     * @returns A future, containing the resolved world instance if exists, otherwise nullptr
     */
    [[nodiscard]] extern Future<ptr<World>> GetWorld(cref<asset_guid> guid_) noexcept;

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
    extern Future<bool> Destroy(mref<ptr<World>> world_);
}
