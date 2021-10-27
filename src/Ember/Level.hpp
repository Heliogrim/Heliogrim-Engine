#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Entity.hpp"

namespace ember {

    class Level {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        Level() noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         */
        Level(_In_ cref<Level> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         */
        Level(_Inout_ mref<Level> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 24.10.2021
         */
        ~Level() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<Level> operator=(_In_ cref<Level> other_) noexcept;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<Level> operator=(_Inout_ mref<Level> other_) noexcept;

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
        _Success_(return == true) bool addEntity(_In_ cref<Entity> entity_);

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
        _Success_(return == true) bool removeEntity(_In_ cref<Entity> entity_);
    };

    namespace level {

        /**
         * Check whether given level has underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @returns level_ The level to check.
         */
        [[nodiscard]] _Success_(return == true) extern bool valid(_In_ cref<Level> level_) noexcept;

        /**
         * Creates a new level with underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @returns A future, containing the newly created level if succeeded, otherwise invalid level.
         */
        [[nodiscard]] _Success_(valid(return.get())) extern future<Level> create() noexcept;

        /**
         * Destroy the given level and underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @param  level_ The Level to destroy.
         *
         * @returns A future, representing whether the entity was successfully destroyed.
         */
        [[nodiscard]] _Success_(return.get() == true) extern future<bool> destroy(_Inout_ mref<Level> level_) noexcept;
    }
}
