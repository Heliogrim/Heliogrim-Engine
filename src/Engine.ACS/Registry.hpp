#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>
#include <Engine.Common/Make.hpp>

#include <Ember/Actor.hpp>

#include "ComponentTypeId.hpp"
#include "Traits.hpp"
#include "Pool.hpp"
#include "PoolWrapper.hpp"
#include "Registry.hpp"

namespace ember {
    /**
     * Forward Declaration
     */
    class Actor;
}

namespace ember::engine::acs {

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
            using wrapper_type = PoolWrapper<ValueType>;

            auto ptr = &pool_type::getOrCreate();

            // Temporary
            const auto typeId { EmberClass::stid<ValueType>() };
            auto mapped = _pools[typeId];

            if (mapped == nullptr) {
                _pools[typeId] = new PoolWrapper<ValueType>(ptr);
            }

            return *ptr;
        }

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
        ska::bytell_hash_map<type_id, ptr<PoolWrapperBase>> _pools;

    public:
        template <class ValueType_, typename... Args_>
        [[nodiscard]] ptr<ValueType_> acquireActorComponent(cref<actor_guid> guid_, Args_&& ...args_) {

            auto& pool { getOrCreatePool<ValueType_>() };
            const auto result = pool.emplace(guid_, _STD forward<Args_>(args_)...);

            return result.second ? result.first : nullptr;
        }

        void releaseActorComponent(cref<actor_guid> guid_, cref<type_id> typeId_);

        template <IsEmberObject ValueType_>
        void releaseActorComponent(cref<actor_guid> guid_, mref<ptr<ValueType_>> value_) {
            // TODO: releaseActorComponent(guid_, value_->getClass()->typeId());
            releaseActorComponent(guid_, value_->getTypeId());
        }

    private:
        ptr<pool_type<Actor>> _defaultActorPool = &pool_type<Actor>::getOrCreate();

    public:
        [[nodiscard]] ptr<Actor> createActor() noexcept;

        void destroyActor(mref<ptr<Actor>> actor_);
    };
}
