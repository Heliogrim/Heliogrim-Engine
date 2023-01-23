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
            const _STD defer_lock_t
        );

    public:
        ManageGuard(cref<this_type>) = delete;

        ManageGuard(mref<this_type> other_) noexcept;

    public:
        virtual ~ManageGuard();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept;

    private:
        ptr<resource_type> _resource;
        ResourceUsageFlags _ownedFlags;

    private:
        void swap(ref<this_type> other_) noexcept {
            _STD swap(_resource, other_._resource);
            _STD swap(_ownedFlags, other_._ownedFlags);
        }

    public:
        [[nodiscard]] bool try_acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) noexcept;

        ref<this_type> acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault);

        ref<this_type> acquire(
            const ptr<resource_type> resource_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        );

        // ReSharper disable once CppHiddenFunction
        ref<this_type> release();

        const ptr<resource_type> reset(const ptr<resource_type> next_);

        const ptr<resource_type> reset(_STD nullptr_t);

    public:
        /**
         * Check whether this manager has an underlying resource
         *
         * @author Julius
         * @date 23.01.2023
         *
         * @returns False if resource is present, otherwise true.
         */
        [[nodiscard]] bool empty() const noexcept;

    public:
        [[nodiscard]] bool owns() const noexcept;

        [[nodiscard]] ResourceUsageFlags owned_flags() const noexcept;

    public:
        [[nodiscard]] operator bool() const noexcept;

    public:
        [[nodiscard]] const non_owning_rptr<const value_type> imm() const noexcept;

        [[nodiscard]] const non_owning_rptr<const value_type> imm() noexcept;

        [[nodiscard]] const ptr<value_type> mut() const noexcept;

        [[nodiscard]] const ptr<value_type> mut() noexcept;
    };
}
