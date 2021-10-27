#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Level.hpp"

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
        ~World() noexcept = delete;

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

    public:
        /**
         * Object allocation operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns The result of the operation.
         */
        template <typename ...Args_>
        ptr<void> operator new(_STD size_t, Args_ ...) = delete;

        /**
         * Object de-allocation operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns The result of the operation.
         */
        template <typename ...Args_>
        void operator delete(ptr<void>, Args_ ...) = delete;

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
    };
}
