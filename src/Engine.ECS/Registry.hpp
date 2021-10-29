#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Make.hpp>

#include "ComponentTypeId.hpp"
#include "Traits.hpp"
#include "Pool.hpp"
#include "Registry.hpp"

namespace ember::engine::ecs {

    // TODO: Replace / Reconcider
    struct __declspec(novtable) pool_adapter {

        void* self;

        virtual void insert(_In_ cref<entity_guid> key_) = 0;

        virtual void* get(_In_ cref<entity_guid> key_) = 0;
    };

    template <IsComponent ComponentType_>
    struct pool_adapter_impl final :
        public pool_adapter {

        pool_adapter_impl() noexcept = default;

        pool_adapter_impl(_In_ ptr<pool<ComponentType_>> pool_) noexcept :
            pool_adapter() {
            self = pool_;
        }

        void insert(cref<entity_guid> key_) override {
            auto* cp { static_cast<pool<ComponentType_>*>(self) };
            cp->insert(key_);
        }

        void* get(cref<entity_guid> key_) override {
            auto* cp { static_cast<pool<ComponentType_>*>(self) };
            return cp->get(key_);
        }
    };

    // TODO: Remove registry and dereference 
    class registry {
    public:
        using this_type = _STD unique_ptr<registry>;
        using size_type = size_t;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 29.10.2021
         */
        ~registry();

    private:
        static this_type _this;

    public:
        /**
         * Destroys this 
         *
         * @author Julius
         * @date 05.09.2020
         */
        static void destroy() noexcept;

        /**
         * Get the internal stored instance of registry
         *
         * @author Julius
         * @date 05.09.2020
         *
         * @returns A reference to a registry.
         */
        [[nodiscard]] static registry& get() noexcept;

        /**
         * Gets a internal stored instance of registry or create one
         *
         * @author Julius
         * @date 05.09.2020
         *
         * @returns A reference to a registry.
         */
        static registry& make() noexcept;

    public:
        /**
         * Pool Operations
         */

        /**
         * Gets or create pool
         *
         * @tparam ComponentType Type of the component type.
         *
         * @returns The or create pool.
         */
        template <IsComponent ComponentType>
        pool<ComponentType>& getOrCreatePool() {
            constexpr type_id typeId = ComponentType::type_id;
            auto ptr = static_cast<ecs::pool<ComponentType>*>(_pools[typeId]);
            if (ptr == nullptr) {
                ptr = &ecs::pool<ComponentType>::getOrCreate();
                _pools[typeId] = ptr;
                _adapters[typeId] = new pool_adapter_impl<ComponentType> { ptr };
            }
            return *ptr;
        }

        /**
         * Get a pool_adapter
         *
         * @param typeId_ The type identifier related to the queried pool
         *
         * @returns A pointer to a acknowledged pool_adapter, otherwise false
         */
        _Success_(return != nullptr) pool_adapter* get_pool_adapter(_In_ cref<component_type_id> typeId_);

        /**
         * Gets the pool
         *
         * @tparam ComponentType Type of the component type.
         *
         * @returns A reference to a clockwork::__internal::ecs::pool&lt;ComponentType&gt;
         */
        template <IsComponent ComponentType>
        pool<ComponentType>& pool() {
            constexpr type_id typeId = ComponentType::type_id;
            return *(static_cast<ecs::pool<ComponentType>*>(_pools[typeId]));
        }

    public:
        /**
         * Component Operations
         */

    private:
        /**
         * Unsafe get of a component reference
         *
         * @tparam ComponentType Type of the component type.
         * @param  key_ The key.
         * @param hash_
         *
         * @returns A reference to a ComponentType.
         */
        template <IsComponent ComponentType>
        ComponentType& unsafe_get(const entity_guid& key_, const typename ecs::pool<ComponentType>::hash_type hash_) {
            return pool<ComponentType>().unsafe_get(key_, hash_);
        }

        /**
         * Unsafe get of component references
         *
         * @tparam First_ Type of the first.
         * @tparam Second_ Type of the second.
         * @tparam Args_ Type of the arguments.
         * @param  key_ The key.
         * @param hash_
         *
         * @returns A tuple&lt;First_&amp;,Second_&amp;,Args_&amp;...&gt;
         */
        template <IsComponent First_, IsComponent Second_, IsComponent... Args_>
        _STD tuple<First_&, Second_&, Args_&...> unsafe_get(const entity_guid& key_,
            const typename ecs::pool<First_>::hash_type hash_) {
            return _STD forward_as_tuple(get<First_>(key_, hash_), get<Second_>(key_, hash_),
                get<Args_>(key_, hash_)...);
        }

        /**
         * Gets a component pointer using the given key
         *
         * @tparam ComponentType Type of the component type.
         * @param  key_ The key.
         * @param hash_
         *
         * @returns Null if it fails, else a pointer to a ComponentType.
         */
        template <IsComponent ComponentType>
        ComponentType* get(const entity_guid& key_, const typename ecs::pool<ComponentType>::hash_type hash_) {
            return pool<ComponentType>().get(key_, hash_);
        }

        /**
         * Gets a tuple&lt; first , second ,args ...&gt; using the given key
         *
         * @tparam First_ Type of the first.
         * @tparam Second_ Type of the second.
         * @tparam Args_ Type of the arguments.
         * @param  key_ The key.
         * @param hash_
         *
         * @returns A tuple&lt;First_,Second_,args_...&gt;
         */
        template <IsComponent First_, IsComponent Second_, IsComponent... Args_>
        _STD tuple<First_*, Second_*, Args_*...> get(const entity_guid& key_,
            const typename ecs::pool<First_>::hash_type hash_) {
            return _STD forward_as_tuple(get<First_>(key_, hash_), get<Second_>(key_, hash_),
                get<Args_>(key_, hash_)...);
        }

    public:
        /**
         * Unsafe get of a component reference
         *
         * @tparam ComponentType Type of the component type.
         * @param  key_ The key.
         *
         * @returns A reference to a ComponentType.
         */
        template <IsComponent ComponentType>
        ComponentType& unsafe_get(const entity_guid& key_) {
            return pool<ComponentType>().unsafe_get(key_);
        }

        /**
         * Unsafe get of component references
         *
         * @tparam First_ Type of the first.
         * @tparam Second_ Type of the second.
         * @tparam Args_ Type of the arguments.
         * @param  key_ The key.
         *
         * @returns A tuple&lt;First_&amp;,Second_&amp;,Args_&amp;...&gt;
         */
        template <IsComponent First_, IsComponent Second_, IsComponent... Args_>
        _STD tuple<First_&, Second_&, Args_&...> unsafe_get(const entity_guid& key_) {
            const typename ecs::pool<First_>::hash_type hash = typename ecs::pool<First_>::assign_hasher()(key_);
            return _STD forward_as_tuple(get<First_>(key_, hash), get<Second_>(key_, hash), get<Args_>(key_, hash)...);
        }

        /**
         * Gets a component pointer using the given key
         *
         * @tparam ComponentType Type of the component type.
         * @param  key_ The key.
         *
         * @returns Null if it fails, else a pointer to a ComponentType.
         */
        template <IsComponent ComponentType>
        ComponentType* get(const entity_guid& key_) {
            return pool<ComponentType>().get(key_);
        }

        /**
         * Gets a tuple&lt; first , second ,args ...&gt; using the given key
         *
         * @tparam First_ Type of the first.
         * @tparam Second_ Type of the second.
         * @tparam Args_ Type of the arguments.
         * @param  key_ The key.
         *
         * @returns A tuple&lt;First_,Second_,args_...&gt;
         */
        template <IsComponent First_, IsComponent Second_, IsComponent... Args_>
        _STD tuple<First_*, Second_*, Args_*...> get(const entity_guid& key_) {
            const typename ecs::pool<First_>::hash_type hash = typename ecs::pool<First_>::assign_hasher()(key_);
            return _STD forward_as_tuple(get<First_>(key_, hash), get<Second_>(key_, hash), get<Args_>(key_, hash)...);
        }

    private:
        ska::bytell_hash_map<type_id, pool_adapter*> _adapters;
        ska::bytell_hash_map<type_id, void*> _pools;
    };
}
