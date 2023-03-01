#pragma once
#include "Resource.hpp"
#include <Engine.Scheduler/Helper/Wait.hpp>

namespace hg::engine::resource {
    template <typename ManagedType_>
    class UniqueResource final :
        public Resource<ManagedType_> {
    public:
        using this_type = UniqueResource<ManagedType_>;
        using underlying_type = Resource<ManagedType_>;

        using value_type = typename underlying_type::value_type;
        using guard_type = TypedManageGuard<value_type>;

    public:
        constexpr UniqueResource() noexcept :
            underlying_type() {}

        constexpr UniqueResource(nullptr_t) noexcept :
            underlying_type() {}

        template <typename Type_ = ManagedType_> requires _STD is_default_constructible_v<Type_>
        constexpr UniqueResource(_STD in_place_t) noexcept(_STD is_nothrow_default_constructible_v<Type_>) :
            underlying_type(_STD in_place) {}

        /**
         * TODO: Check whether we want to hide this constructor with a `Ty_ make_*(...)` function
         */
        constexpr UniqueResource(__restricted_ptr<value_type> value_) noexcept :
            underlying_type(value_) {}

        template <typename... Args_> requires _STD is_constructible_v<value_type, Args_...>
        constexpr UniqueResource(Args_&&... args_) noexcept (_STD is_nothrow_constructible_v<value_type, Args_...>) :
            underlying_type(_STD forward<Args_>(args_)...) {}

        ~UniqueResource() override = default;

    private:
        _STD atomic_flag _lck;

    public:
        [[nodiscard]] ManageGuard acquire(const ResourceUsageFlags flags_) override {
            while (_lck.test_and_set(_STD memory_order::release)) {
                scheduler::waitOnAtomic(_lck, true);
            }
            return guard_type { this, flags_, _STD adopt_lock };
        }

        [[nodiscard]] bool try_acquire(
            _Out_ ref<ManageGuard> guard_,
            const ResourceUsageFlags flags_
        ) noexcept override {
            if (_lck.test_and_set(_STD memory_order::consume)) {
                return false;
            }

            guard_ = guard_type { this, flags_, _STD adopt_lock };
            return true;
        }

    protected:
        void release(const ResourceUsageFlags flags_) override {
            _lck.clear(_STD memory_order::release);
            _lck.notify_one();
        }
    };
}
