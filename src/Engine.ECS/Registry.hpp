#pragma once
#include "Traits.hpp"
#include "Pool.hpp"

namespace ember::engine::ecs {
    // TODO: Remove registry and dereference 
    class registry {
    public:
        using this_type = _STD unique_ptr<registry>;
        using size_type = size_t;

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
            }
            return *ptr;
        }

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
        static this_type _this;
        _STD unordered_map<type_id, void*> _pools;
    };
}
