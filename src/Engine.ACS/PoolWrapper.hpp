#pragma once

#include "Pool.hpp"
#include "Traits.hpp"

namespace ember::engine::acs {

    class __declspec(novtable) PoolWrapperBase {
    public:
        virtual void erase(cref<actor_guid> key_) = 0;
    };

    template <typename PooledType_>
    class PoolWrapper final :
        public PoolWrapperBase {
    public:
        using pool_type = Pool<actor_guid, PooledType_, invalid_actor_guid>;

    public:
        PoolWrapper(const ptr<pool_type> actual_) noexcept :
            _actual(actual_) {}

    private:
        ptr<pool_type> _actual;

    public:
        [[nodiscard]] const ptr<pool_type> pool() const noexcept {
            return _actual;
        }

    public:
        void erase(cref<actor_guid> key_) override {
            _actual->erase(key_);
        }
    };

}
