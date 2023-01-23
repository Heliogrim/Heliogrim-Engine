#pragma once

#include <mutex>

#include "../ResourceUsageFlag.hpp"

namespace ember::engine::resource {
    class ResourceBase;
}

namespace ember::engine::resource {
    class __declspec(novtable) ManageGuard {
    public:
        using this_type = ManageGuard;

        using resource_type = ResourceBase;
        using value_type = void;

    protected:
        constexpr ManageGuard() noexcept;

        ManageGuard(const ptr<resource_type> resource_);

        ManageGuard(const ptr<resource_type> resource_, const ResourceUsageFlags flags_);

        ManageGuard(
            const ptr<resource_type> resource_,
            const ResourceUsageFlags flags_,
            const _STD defer_lock_t);

    public:
        ManageGuard(cref<this_type>) = delete;

        ManageGuard(mref<this_type> other_) noexcept;

    public:
        virtual ~ManageGuard();

    private:
        ptr<resource_type> _resource;
        ResourceUsageFlags _flags;

    public:
        [[nodiscard]] bool try_acquire() noexcept;

        ref<this_type> acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault);

        ref<this_type> acquire(
            const ptr<resource_type> resource_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        );

        ref<this_type> release();

    public:
        [[nodiscard]] bool owns() const noexcept;

        [[nodiscard]] operator bool() const noexcept;

    public:
        [[nodiscard]] ResourceUsageFlags owned_flags() const noexcept;

    public:
        [[nodiscard]] const non_owning_rptr<const value_type> imm() const noexcept;

        [[nodiscard]] const non_owning_rptr<const value_type> imm() noexcept;

        [[nodiscard]] const ptr<value_type> mut() const noexcept;

        [[nodiscard]] const ptr<value_type> mut() noexcept;
    };
}
