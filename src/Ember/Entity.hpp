#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.ECS/Traits.hpp>

#include "Inbuilt.hpp"

#include "Traits.hpp"

namespace ember {

    namespace {
        /**
         * An entity base.
         *
         * @author Julius
         * @date 20.08.2021
         */
        class EntityBase {
        public:
            /**
             * Default constructor
             *
             * @author Julius
             * @date 20.08.2021
             */
            EntityBase() = delete;

            /**
             * Destructor
             *
             * @author Julius
             * @date 20.08.2021
             */
            ~EntityBase() noexcept = default;

        protected:
            /**
             * Entity Id
             */
            engine::ecs::entity_guid _guid;

        public:
            /**
             * Gets the unique identifier
             *
             * @author Julius
             * @date 07.01.2021
             *
             * @returns A cref&lt;engine::ecs::entity_guid&gt;
             */
            [[nodiscard]] cref<engine::ecs::entity_guid> guid() const noexcept;

            /**
             * Gets the unique identifier
             *
             * @author Julius
             * @date 07.01.2021
             *
             * @returns An engine::ecs::entity_guid.
             */
            [[nodiscard]] engine::ecs::entity_guid guid() noexcept;

            /**
             * Cast that converts the given to an entity_guid
             *
             * @author Julius
             * @date 07.01.2021
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] operator ember::engine::ecs::entity_guid() const noexcept;

            /**
             * Cast that converts the given to an entity_guid
             *
             * @author Julius
             * @date 07.01.2021
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] operator ember::engine::ecs::entity_guid() noexcept;

        protected:
            /**
             * Components
             */

        public:
            /**
             * Gets a component from internal storage related to entity
             *
             * @returns A ref&lt;const Ty&gt; which is a immutable reference to a component
             */
            template <class Ty>
            [[nodiscard]] ref<const Ty> get() const;

            /**
             * Gets a component from internal storage related to entity
             *
             * @returns A ref&lt;Ty&gt; which is a mutable reference to a component
             */
            template <class Ty>
            [[nodiscard]] ref<Ty> get();

            /**
             * Check whether entity has component
             *
             * @returns True if it succeeds, false if it fails.
             */
            template <class Ty>
            [[nodiscard]] bool has() const noexcept;

            /**
             * Check whether entity has component
             *
             * @returns True if it succeeds, false if it fails.
             */
            template <class Ty>
            [[nodiscard]] bool has() noexcept;

            /**
             * Records a component to internal storage
             *
             * @returns A ref&lt;Ty&gt; which is mutable reference to a component
             */
            template <class Ty>
            [[nodiscard]] ref<Ty> record() noexcept;

            /**
             * Records a component to internal storage
             *
             * @param  obj_ Source Component to clone
             *
             * @returns A ref&lt;Ty&gt; which is mutable reference to a component.
             */
            template <class Ty>
            [[nodiscard]] ref<Ty> record(cref<Ty> obj_) noexcept;

            /**
             * Removes a component from internal storage
             *
             * @returns True if it succeeds, false if it fails.
             */
            template <class Ty>
            [[nodiscard]] bool remove() noexcept;
        };
    }

    class Entity final :
        public EntityBase {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.08.2021
         */
        Entity() noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         */
        Entity(_In_ cref<Entity> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         */
        Entity(_Inout_ mref<Entity> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.08.2021
         */
        ~Entity() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The source entity.
         *
         * @returns A shallow copy of this.
         */
        ref<Entity> operator=(_In_ cref<Entity> other_) noexcept;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The source entity.
         *
         * @returns A shallow copy of this.
         */
        ref<Entity> operator=(_Inout_ mref<Entity> other_) noexcept;
    };

    namespace entity {
        /**
         * Check whether given entity has underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @param  entity_ The entity to check.
         */
        [[nodiscard]] _Success_(return == true) extern bool valid(_In_ cref<Entity> entity_) noexcept;

        /**
         * Creates a new entity with underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @returns A future, containing the newly created entity if succeeded, otherwise invalid entity.
         */
        [[nodiscard]] _Success_(valid(return.get())) extern future<Entity> create() noexcept;

        /**
         * Create a new entity with underlying object equivalent to given entity_
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @param  entity_ The entity to clone from.
         *
         * @returns A future, containing the newly created entity if succeeded, otherwise invalid entity.
         */
        [[nodiscard]] _Success_(valid(return.get())) extern future<Entity> clone(_In_ cref<Entity> entity_) noexcept;

        /**
         * Destroy the given entity and underlying object
         *
         * @author Julius
         * @date 25.10.2021
         *
         * @param  entity_ The entity to destroy.
         *
         * @returns A future, representing whether the entity was successfully destroyed.
         */
        [[nodiscard]] _Success_(return.get() == true) extern future<bool> destroy(_Inout_ mref<Entity> entity_) noexcept;
    }
}
