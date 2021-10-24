#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Entity.hpp"

namespace ember::world {

    class Level {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        Level() = delete;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        Level(cref<Level>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        Level(mref<Level>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        ~Level() noexcept = delete;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns A shallow copy of this.
         */
        ref<Level> operator=(cref<Level>) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @returns A shallow copy of this.
         */
        ref<Level> operator=(mref<Level>) noexcept = delete;

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
         * Static instantiation of a new level
         *
         * @author Julius
         * @date 24.10.2021
         */
        [[nodiscard]] _Success_(return != nullptr) static ptr<Level> create();

        /**
         * Static destruction of the given level
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  level_ The level to destroy.
         */
        _Success_(return == true) static bool destroy(_Inout_ const ptr<Level> level_);

    public:
        /**
         * Tries to add the given entity to this level
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  entity_ The entity to add.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool addEntity(_In_ const ptr<Entity> entity_);

        /**
         * Removes the entity from this level
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  entity_ The entity to remove.
         *
         * @returns True if it succeeds, false if it fails.
         */
        _Success_(return == true) bool removeEntity(_In_ const ptr<Entity> entity_);
    };
}
