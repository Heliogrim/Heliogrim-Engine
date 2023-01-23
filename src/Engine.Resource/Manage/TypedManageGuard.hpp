#pragma once

#include "__fwd.hpp"
#include "ManageGuard.hpp"

namespace ember::engine::resource {
    template <typename Type_>
    concept IsManageType = true;

    template <IsManageType ValueType_>
    class TypedManageGuard :
        public ManageGuard {
    public:
        template <typename>
        friend class MutTypedManageGuard;

    public:
        using this_type = TypedManageGuard<ValueType_>;

        using value_type = ValueType_;

        using ManageGuard::resource_type;
        using typed_resource_type = Resource<ValueType_>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 27.08.2021
         */
        constexpr TypedManageGuard() noexcept = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param resource_ The resource to guard.
         */
        [[nodiscard]] TypedManageGuard(const ptr<typed_resource_type> resource_) :
            ManageGuard(resource_, ResourceUsageFlag::eNone) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param resource_ The resource to guard.
         * @param flags_ The guarding flags to apply while acquiring.
         */
        TypedManageGuard(const ptr<typed_resource_type> resource_, const ResourceUsageFlags flags_) :
            ManageGuard(resource_, flags_) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param resource_ The resource to guard.
         * @param flags_ The guarding flags to apply while acquiring.
         */
        TypedManageGuard(
            const ptr<typed_resource_type> resource_,
            const ResourceUsageFlags flags_,
            const _STD defer_lock_t) :
            ManageGuard(resource_, flags_, _STD defer_lock) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 28.08.2021
         */
        TypedManageGuard(cref<this_type>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  other_ The other.
         */
        [[nodiscard]] TypedManageGuard(mref<this_type> other_) noexcept :
            ManageGuard(_STD move(other_)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 28.08.2021
         */
        ~TypedManageGuard() noexcept override = default;

    public:
        ref<this_type> acquire(
            const ptr<typed_resource_type> resource_,
            const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault
        ) {
            return ManageGuard::acquire(resource_, flags_);
        }

        ref<this_type> release() {
            return ManageGuard::release();
        }

    public:
        [[nodiscard]] const non_owning_rptr<const value_type> imm() const noexcept {
            return static_cast<const ptr<const value_type>>(ManageGuard::imm());
        }

        [[nodiscard]] const non_owning_rptr<const value_type> imm() noexcept {
            return static_cast<const ptr<const value_type>>(ManageGuard::imm());
        }

        [[nodiscard]] const ptr<value_type> mut() const noexcept {
            return static_cast<const ptr<value_type>>(ManageGuard::mut());
        }

        [[nodiscard]] const ptr<value_type> mut() noexcept {
            return static_cast<const ptr<value_type>>(ManageGuard::mut());
        }

    public:
        [[nodiscard]] const non_owning_rptr<const value_type> operator->() const noexcept {
            return this_type::imm();
        }

        [[nodiscard]] const non_owning_rptr<const value_type> operator->() noexcept {
            return this_type::imm();
        }

        [[nodiscard]] cref<value_type> operator*() const noexcept {
            return *this_type::imm();
        }

        [[nodiscard]] cref<value_type> operator*() {
            return *this_type::imm();
        }
    };

    template <typename ManagedType_>
    class MutTypedManageGuard final :
        public TypedManageGuard<ManagedType_> {
    public:
        using this_type = MutTypedManageGuard<ManagedType_>;
        using underlying_type = TypedManageGuard<ManagedType_>;

        using underlying_type::value_type;

    public:
        using underlying_type::this_type;

    public:
        [[nodiscard]] const ptr<typename value_type> operator->() const noexcept {
            return underlying_type::mut();
        }

        [[nodiscard]] ref<typename value_type> operator*() const noexcept {
            return *underlying_type::mut();
        }
    };
}
