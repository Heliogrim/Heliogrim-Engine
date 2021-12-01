#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Make.hpp>
#include <Ember/Actor.hpp>

#include "ComponentTypeId.hpp"
#include "Traits.hpp"
#include "Pool.hpp"
#include "Registry.hpp"

namespace ember::engine::acs {

    // TODO: Replace / Reconcider
    struct __declspec(novtable) pool_adapter {

        void* self;

        virtual void insert(_In_ cref<actor_guid> key_) = 0;

        virtual void* get(_In_ cref<actor_guid> key_) = 0;

        virtual void erase(_In_ cref<actor_guid> key_) = 0;
    };

    template <typename PoolType_>
    struct pool_adapter_impl final :
        public pool_adapter {

        pool_adapter_impl() noexcept = default;

        pool_adapter_impl(_In_ ptr<PoolType_> pool_) noexcept :
            pool_adapter() {
            self = pool_;
        }

        void insert(cref<actor_guid> key_) override {
            auto* cp { static_cast<PoolType_*>(self) };
            cp->insert(key_);
        }

        void* get(cref<actor_guid> key_) override {
            auto* cp { static_cast<PoolType_*>(self) };
            return cp->get(key_);
        }

        void erase(cref<actor_guid> key_) override {
            auto* cp { static_cast<PoolType_*>(self) };
            cp->erase(key_);
        }
    };

    class Registry {
    public:
        using size_type = size_t;

        template <typename PooledType_>
        using pool_type = Pool<actor_guid, PooledType_, invalid_actor_guid>;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 29.10.2021
         */
        ~Registry();

    public:
        /**
         * Pool Operations
         */

        /**
         * Gets or create pool
         *
         * @tparam ValueType Type of the value to get a pool of.
         *
         * @returns The or create pool.
         */
        template <typename ValueType>
        pool_type<ValueType>& getOrCreatePool() {

            using pool_type = pool_type<ValueType>;

            auto ptr = &pool_type::getOrCreate();

            // Temporary
            type_id typeId { reinterpret_cast<u64>(ptr) };
            auto mapped = static_cast<pool_type*>(_pools[typeId]);

            if (mapped == nullptr) {
                _pools[typeId] = ptr;
                _adapters[typeId] = new pool_adapter_impl<pool_type> { ptr };
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
         * @tparam ValueType Type of the value to get a pool of.
         *
         * @returns A reference to a pool of value type.
         */
        template <typename ValueType>
        pool_type<ValueType>& pool() {

            using pool_type = pool_type<ValueType>;
            auto* ptr = pool_type::get();

            return *ptr;
        }

    public:
        /**
         * Unsafe get of a value reference
         *
         * @tparam ValueType Type of the value.
         * @param  key_ The key.
         *
         * @returns A reference to a value.
         */
        template <typename ValueType>
        ValueType& unsafe_get(const actor_guid& key_) {
            return pool<ValueType>().unsafe_get(key_);
        }

        /**
         * Unsafe get of value references
         *
         * @tparam First_ Type of the first.
         * @tparam Second_ Type of the second.
         * @tparam Args_ Type of the arguments.
         * @param  key_ The key.
         *
         * @returns A tuple&lt;First_&amp;,Second_&amp;,Args_&amp;...&gt;
         */
        template <typename First_, typename Second_, typename... Args_>
        _STD tuple<First_&, Second_&, Args_&...> unsafe_get(const actor_guid& key_) {

            using pool_type = pool_type<First_>;

            const typename pool_type::hash_type hash = typename pool_type::assign_hasher()(key_);
            return _STD forward_as_tuple(get<First_>(key_, hash), get<Second_>(key_, hash), get<Args_>(key_, hash)...);
        }

        /**
         * Gets a value pointer using the given key
         *
         * @tparam ValueType Type of the value to get.
         * @param  key_ The key.
         *
         * @returns Null if it fails, else a pointer to a value.
         */
        template <typename ValueType>
        ValueType* get(const actor_guid& key_) {
            return pool<ValueType>().get(key_);
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
        template <typename First_, typename Second_, typename... Args_>
        _STD tuple<First_*, Second_*, Args_*...> get(const actor_guid& key_) {

            using pool_type = pool_type<First_>;

            const typename pool_type::hash_type hash = typename pool_type::assign_hasher()(key_);
            return _STD forward_as_tuple(get<First_>(key_, hash), get<Second_>(key_, hash), get<Args_>(key_, hash)...);
        }

    private:
        ska::bytell_hash_map<type_id, pool_adapter*> _adapters;
        ska::bytell_hash_map<type_id, void*> _pools;

    private:
        ptr<pool_type<Actor>> _defaultActorPool = &pool_type<Actor>::getOrCreate();

    public:
        [[nodiscard]] ptr<Actor> createActor() noexcept {

            const auto guid = generate_actor_guid();

            _defaultActorPool->insert(guid, Actor {});
            return _defaultActorPool->get(guid);
        }
    };
}
