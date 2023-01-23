#pragma once
#include "Resource.hpp"
#include <Engine.Scheduler/Helper/Wait.hpp>

namespace ember::engine::resource {
    template <typename ManagedType_>
    class UniqueResource final :
        public Resource<ManagedType_> {
    public:
        using this_type = UniqueResource<ManagedType_>;
        using underlying_type = Resource<ManagedType_>;

        using underlying_type::value_type;
        using guard_type = TypedManageGuard<typename value_type>;

    public:
        using underlying_type::this_type;

    private:
        _STD atomic_flag _lck;

    public:
        [[nodiscard]] ManageGuard acquire(const ResourceUsageFlags flags_) override {
            while (_lck.test_and_set(_STD memory_order::release)) {
                scheduler::waitOnAtomic(_lck, true);
            }
            return guard_type { this, flags_ };
        }

        [[nodiscard]] bool try_acquire(
            _Out_ ref<ManageGuard> guard_,
            const ResourceUsageFlags flags_
        ) noexcept override {
            if (_lck.test_and_set(_STD memory_order::consume)) {
                return false;
            }

            guard_ = guard_type { this, flags_ };
            return true;
        }

    protected:
        void release(const ResourceUsageFlags flags_) override {
            _lck.clear(_STD memory_order::release);
            _lck.notify_one();
        }
    };
}
