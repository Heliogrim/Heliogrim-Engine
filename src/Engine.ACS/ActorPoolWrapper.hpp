#pragma once

#include <Heliogrim/Actor.hpp>
#include "Pool.hpp"
#include "Traits.hpp"

namespace hg::engine::acs {
    class __declspec(novtable) ActorPoolWrapperBase {
    public:
        virtual void insert(cref<actor_guid> key_, cref<ActorInitializer> initializer_) = 0;

        virtual ptr<Actor> get(cref<actor_guid> key_) = 0;

        virtual void erase(cref<actor_guid> key_) = 0;
    };

    template <typename PooledType_>
    class ActorPoolWrapper final :
        public ActorPoolWrapperBase {
    public:
        using pool_type = Pool<actor_guid, PooledType_, invalid_actor_guid>;

    public:
        ActorPoolWrapper(const ptr<pool_type> actual_) noexcept :
            _actual(actual_) {}

    private:
        ptr<pool_type> _actual;

    public:
        [[nodiscard]] const ptr<pool_type> pool() const noexcept {
            return _actual;
        }

    public:
        void insert(cref<actor_guid> key_, cref<ActorInitializer> initializer_) override {
            _actual->emplace(key_, initializer_);
        }

        ptr<Actor> get(cref<actor_guid> key_) override {
            return static_cast<ptr<Actor>>(_actual->get(key_));
        }

        void erase(cref<actor_guid> key_) override {
            _actual->erase(key_);
        }
    };
}
