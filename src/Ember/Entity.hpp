#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.ECS/Traits.hpp>

#include "Inbuilt.hpp"
#include "Engine.Common/Math/Transformation.hpp"

namespace ember {

    /**
     * Forward Declaration
     */
    class Entity;

    /**
     * Creates a new entity with underlying object
     *
     * @author Julius
     * @date 25.10.2021
     *
     * @returns A future, containing the newly created entity if succeeded, otherwise invalid entity.
     */
    [[nodiscard]] extern future<Entity> CreateEntity() noexcept;

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
    [[nodiscard]] extern future<Entity> CloneEntity(cref<Entity> entity_) noexcept;

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
    [[nodiscard]] extern future<bool> Destroy(mref<Entity> entity_) noexcept;

    /**
     * Check whether given entity has underlying object
     *
     * @author Julius
     * @date 25.10.2021
     *
     * @param  entity_ The entity to check.
     */
    [[nodiscard]] extern bool Valid(cref<Entity> entity_) noexcept;

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
            _guid(invalid_entity_guid) {}

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

    protected:
        explicit EntityBase(cref<entity_guid> guid_) noexcept;

    public:
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
        entity_guid _guid;

    public:
        /**
         * Gets the unique identifier
         *
         * @author Julius
         * @date 28.10.2021
         *
         * @returns An engine::ecs::entity_guid.
         */
        [[nodiscard]] entity_guid guid() const noexcept;

    protected:
        /**
         * Components
         */

    protected:
        /**
         * Query for a component recorded by this entity
         *
         * @param componentTypeId_ The type identifier for the component to query for.
         *
         * @returns A pointer to the component if it exists, otherwise nullptr.
         */
        [[nodiscard]] ptr<void> get(cref<component_type_id> componentTypeId_) const noexcept;

    public:
        /**
         * Gets a component from internal storage related to entity
         *
         * @returns A ref&lt;const Ty&gt; which is a immutable reference to a component
         */
        template <class ComponentType_>
        [[nodiscard]] const _STD remove_cvref_t<ComponentType_> get() const {
            #ifdef _DEBUG
            auto* ptr = get(ComponentType_::type_id);
            DEBUG_ASSERT(ptr != nullptr, "")
            return ComponentType_ { ptr };
            #else
            return ComponentType_ {get(ComponentType_::type_id)};
            #endif
        }

        /**
         * Gets a component from internal storage related to entity
         *
         * @returns A ref&lt;Ty&gt; which is a mutable reference to a component
         */
        template <class ComponentType_>
        [[nodiscard]] _STD remove_cvref_t<ComponentType_> get() {
            #ifdef _DEBUG
            auto* ptr = get(ComponentType_::type_id);
            DEBUG_ASSERT(ptr != nullptr, "")
            return ComponentType_ { ptr };
            #else
            return ComponentType_ {get(ComponentType_::type_id)};
            #endif
        }

    protected:
        /**
         * Check whether this entity has a certain component recorded
         *
         * @param componentTypeId_ The type identifier for the internal component.
         *
         * @returns True is component is present, false if not.
         */
        [[nodiscard]] bool has(cref<component_type_id> componentTypeId_) const noexcept;

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

    protected:
        /**
         * Records a internal component related to this entity
         *
         * @param componentTypeId_ The type identifier for the internal component.
         */
        void record(cref<component_type_id> componentTypeId_) noexcept;

    public:
        /**
         * Records a component to internal storage
         *
         * @returns A ref&lt;Ty&gt; which is mutable reference to a component
         */
        template <typename ComponentType_> requires HasStaticType<ComponentType_, ember::component_type_id>
        [[nodiscard]] _STD remove_cvref_t<ComponentType_> record() noexcept {
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
        [[nodiscard]] _STD remove_cvref_t<ComponentType_> record(mref<ComponentType_> obj_) noexcept {
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
        friend future<Entity> CreateEntity() noexcept;

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
        Entity(cref<Entity> other_) noexcept;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 24.10.2021
         *
         * @param  other_ The other.
         */
        Entity(mref<Entity> other_) noexcept;

    protected:
        explicit Entity(cref<entity_guid> guid_) noexcept;

    public:
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
        ref<Entity> operator=(cref<Entity> other_) noexcept;

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
        ref<Entity> operator=(mref<Entity> other_) noexcept;

    public:
        [[nodiscard]] cref<math::Transformation> transform() const noexcept;

        void setTransform(cref<math::Transformation> transformation_);
    };
}
