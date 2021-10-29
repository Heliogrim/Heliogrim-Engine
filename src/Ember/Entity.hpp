#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.ECS/Traits.hpp>

#include "Inbuilt.hpp"

namespace ember {

    /**
     * An entity base.
     *
     * @author Julius
     * @date 20.08.2021
     */
    class EntityBase {
    public:
        using this_type = EntityBase;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.08.2021
         */
        EntityBase() noexcept :
            _guid(engine::ecs::invalid_entity_guid) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  other_ The other.
         */
        EntityBase(cref<EntityBase> other_) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @param  other_ The other.
         */
        EntityBase(mref<EntityBase> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.08.2021
         */
        ~EntityBase() noexcept = default;

    protected:
        /**
         * The entity identifier
         */
        engine::ecs::entity_guid _guid;

    public:
        /**
         * Gets the unique identifier
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @returns An engine::ecs::entity_guid.
         */
        [[nodiscard]] engine::ecs::entity_guid guid() const noexcept;

    protected:
        /**
         * Components
         */

    private:
        /**
         * Query for a component recorded by this entity
         *
         * @param componentTypeId_ The type identifier for the component to query for.
         *
         * @returns A pointer to the component if it exists, otherwise nullptr.
         */
        [[nodiscard]] _Success_(return != nullptr) ptr<void> get(
            _In_ cref<component_type_id> componentTypeId_) const noexcept;

    public:
        /**
         * Gets a component from internal storage related to entity
         *
         * @returns A ref&lt;const Ty&gt; which is a immutable reference to a component
         */
        template <class ComponentType_>
        [[nodiscard]] ref<const _STD remove_cvref_t<ComponentType_>> get() const {
            #ifdef _DEBUG
            auto* ptr = get(ComponentType_::type_id);
            DEBUG_ASSERT(ptr != nullptr, "")
            return *static_cast<ComponentType_>(ptr);
            #else
            return *static_cast<ComponentType_>(get(ComponentType_::type_id));
            #endif
        }

        /**
         * Gets a component from internal storage related to entity
         *
         * @returns A ref&lt;Ty&gt; which is a mutable reference to a component
         */
        template <class ComponentType_>
        [[nodiscard]] ref<_STD remove_cvref_t<ComponentType_>> get() {
            #ifdef _DEBUG
            auto* ptr = get(ComponentType_::type_id);
            DEBUG_ASSERT(ptr != nullptr, "")
            return *static_cast<ComponentType_*>(ptr);
            #else
            return *static_cast<ComponentType_>(get(ComponentType_::type_id));
            #endif
        }

    private:
        /**
         * Check whether this entity has a certain component recorded
         *
         * @param componentTypeId_ The type identifier for the internal component.
         *
         * @returns True is component is present, false if not.
         */
        [[nodiscard]] _Success_(return == true) bool has(
            _In_ cref<component_type_id> componentTypeId_) const noexcept;

    public:
        /**
         * Check whether entity has component
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <class ComponentType_>
        [[nodiscard]] bool has() const noexcept {
            return has(ComponentType_::type_id);
        }

        /**
         * Check whether entity has component
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <class ComponentType_>
        [[nodiscard]] bool has() noexcept {
            return has(ComponentType_::type_id);
        }

    private:
        /**
         * Records a internal component related to this entity
         *
         * @param componentTypeId_ The type identifier for the internal component.
         */
        void record(_In_ cref<component_type_id> componentTypeId_) noexcept;

    public:
        /**
         * Records a component to internal storage
         *
         * @returns A ref&lt;Ty&gt; which is mutable reference to a component
         */
        template <typename ComponentType_> requires HasStaticType<ComponentType_, ember::component_type_id>
        [[nodiscard]] ref<_STD remove_cvref_t<ComponentType_>> record() noexcept {
            record(ComponentType_::type_id);
            return get<ComponentType_>();
        }

        /**
         * Records a component to internal storage
         *
         * @param  obj_ Source Component to use
         *
         * @returns A ref&lt;Ty&gt; which is mutable reference to a component.
         */
        template <class ComponentType_>
        [[nodiscard]] ref<_STD remove_cvref_t<ComponentType_>> record(_Inout_ mref<ComponentType_> obj_) noexcept {
            // record(ComponentType_::type_id, &obj_);
            throw NotImplementedException();
            return get<ComponentType_>();
        }

        /**
         * Removes a component from internal storage
         *
         * @returns True if it succeeds, false if it fails.
         */
        template <class ComponentType_>
        [[nodiscard]] bool remove() noexcept = delete;
    };

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
        [[nodiscard]] _Success_(return.get() == true) extern future<bool>
        destroy(_Inout_ mref<Entity> entity_) noexcept;
    }
}
