#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.ECS/Traits.hpp>

#include "../Traits.hpp"

namespace ember::world {

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
        Entity() = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.08.2021
         */
        ~Entity() noexcept = default;

        /**
         * Object allocation operator
         *
         * @author Julius
         * @date 20.08.2021
         *
         * @param  size_ The size.
         *
         * @returns The result of the operation.
         */
        template <typename ...Args_>
        ptr<void> operator new(_STD size_t size_, Args_ ...) = delete;

        /**
         * Object de-allocation operator
         *
         * @author Julius
         * @date 20.08.2021
         *
         * @param  dst_ Destination for the.
         *
         * @returns The result of the operation.
         */
        template <typename ...Args_>
        void operator delete(ptr<void> dst_, Args_ ...) = delete;
    };
}
