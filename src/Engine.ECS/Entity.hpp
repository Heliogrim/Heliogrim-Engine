#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "Traits.hpp"
#include "Component.hpp"
#include "Pool.hpp"

namespace ember::engine::ecs {

    /**
     * The Entity base class
     *
     * @author Julius
     * @date 05.09.2020
     */
    class Entity {
    public:
        using value_type = Entity;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 31.08.2020
         */
        ~Entity() = default;

        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 04.09.2020
         *
         * @returns The identifier.
         */
        const entity_guid& get_guid() const;

        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 04.09.2020
         *
         * @returns The identifier.
         */
        entity_guid get_guid();

        /**
         * Cast that converts the given  to an entity_guid
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns The result of the operation.
         */
        operator entity_guid() const {
            return _guid;
        }

        /**
         * Cast that converts the given  to an entity_guid
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns The result of the operation.
         */
        operator entity_guid() {
            return _guid;
        }

        /**
         * Unsafe get for component request
         *	Will cause error if component was not recorded
         *
         * @tparam ComponentType Type of the component type.
         *
         * @returns A const component_handle&lt;ComponentType&gt;
         */
        template <IsComponent ComponentType>
        const component_handle<ComponentType> unsafe_get() const {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            return component_handle<ComponentType>(pref.unsafe_get(*this));
        }

        /**
         * Unsafe get for component request
         *	Will cause error if component was not recorded
         *
         * @tparam ComponentType Type of the component type.
         *
         * @returns A component_handle&lt;ComponentType&gt;
         */
        template <IsComponent ComponentType>
        component_handle<ComponentType> unsafe_get() {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            return component_handle<ComponentType>(pref.unsafe_get(*this));
        }

        /**
         * Gets a component of type ComponentType from entity
         *
         * @tparam ComponentType Type of the component to resolved.
         *
         * @returns A const component_handle<ComponentType>;
         */
        template <IsComponent ComponentType>
        const component_handle<ComponentType> get() const {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            return component_handle<ComponentType>(pref ? pref.get(*this) : nullptr);
        }

        /**
         * Gets a component of type ComponentType from entity
         *
         * @tparam ComponentType Type of the component to resolved.
         *
         * @returns A component_handle<ComponentType>;
         */
        template <IsComponent ComponentType>
        component_handle<ComponentType> get() {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            return component_handle<ComponentType>(pref ? pref.get(*this) : nullptr);
        }

        /**
         * Constructs a entity structure by given unique identifier
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param  guid_ Unique identifier.
         *
         * @returns An Entity.
         */
        static Entity from(const entity_guid& guid_) noexcept {
            return Entity(guid_);
        }

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 31.08.2020
         */
        Entity();

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.10.2020
         *
         * @param  entityId_ Identifier for the entity.
         */
        Entity(const entity_guid& entityId_);

    private:
        entity_guid _guid;
    };

    /**
     * The dynamic Entity base, providing functions to record and instantiate components
     *
     * @author Julius
     * @date 05.09.2020
     */
    class DynamicEntity :
        public Entity {
    public:
        using base_type = Entity;

        using value_type = DynamicEntity;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Records a default constructed component of type Ty
         *
         * @tparam ComponentType Type of the component to record.
         */
        template <IsComponent ComponentType>
        void record() {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            pref.insert(*this);
        }

        /**
         * Records a move constructed component of type ComponentType
         *
         * @tparam ComponentType Type of the component to record.
         * @param  component_ The component.
         */
        template <IsComponent ComponentType>
        void record(ComponentType&& component_) {
            typename pool<ComponentType>::reference_type pref = pool<ComponentType>::get();
            pref.insert(*this, _STD forward<ComponentType>(component_));
        }

        /**
         * Removes the given component_
         *
         * @author Julius
         * @date 04.09.2020
         *
         * @param  component_ The component to remove.
         */
        template <IsComponent Ty>
        void remove(const Ty& component_) = delete;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.09.2020
         */
        DynamicEntity() = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.10.2020
         *
         * @param  entityId_ Identifier for the entity.
         */
        DynamicEntity(const entity_guid& entityId_);
    };

    /**
     * A template Entity
     *
     * @author Julius
     * @date 05.09.2020
     *
     * @tparam Components Type of the components provided by the entity.
     */
    template <IsComponent Component, IsComponent... Components>
    class TemplateEntity :
        public DynamicEntity {
    protected:
        using base_type = DynamicEntity;

        using value_type = TemplateEntity<Component, Components...>;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.09.2020
         */
        TemplateEntity() {
            // Comment: Should actually resolve as invalid components aka. null components, cause not entity_id ~ invalid_entity_id is provided
            // record_unpack<Component, Components...>();
        }

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.10.2020
         *
         * @param  entityId_ Identifier for the entity.
         */
        TemplateEntity(const entity_guid& entityId_) :
            DynamicEntity(entityId_) {
            record_unpack<Component, Components...>();
        }

        /**
         * Constructor
         *
         * @author Julius
         * @date 05.09.2020
         *
         * @param 		   entityId_ Identifier for the entity.
         * @param [in,out] component_ The first component of the entity to construct.
         * @param [in,out] args_ The packed list of Components ...args_.
         */
        TemplateEntity(const entity_guid& entityId_, Component&& component_, Components&& ...args_) :
            DynamicEntity(entityId_) {
            record<Component>(_STD forward<Component>(component_));
            (record(_STD forward<Components>(args_)), ...);
        }

    private:
        /**
         * Record a template packed parameter of components by record<_First>
         *
         * @tparam _First Type of the first.
         */
        template <typename _First>
        FORCEINLINE void record_unpack() {
            DynamicEntity::record<_First>();
        }

        /**
         * Record a template packed parameter of components by record<Args...>
         *
         * @tparam _First Type of the first.
         * @tparam _Second Type of the second.
         * @tparam Pack Type of the pack.
         */
        template <typename _First, typename _Second, typename... Pack>
        FORCEINLINE void record_unpack() {
            DynamicEntity::record<_First>();
            value_type::template record_unpack<_Second, Pack...>();
        }

    public:
        static value_type make() {
            return _STD forward<value_type>(value_type(generate_entity_guid()));
        }
    };
}
