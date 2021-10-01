#pragma once

#include <mutex>
#include <Engine.Common/Wrapper.hpp>

#include "../ResourceUsageFlag.hpp"

namespace ember::engine::res {

    template <typename Type_>
    concept IsManageType = true;

    template <IsManageType ManageType_>
    class ManageGuard {
    public:
        using value_type = ManageType_;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using this_type = ManageGuard<ManageType_>;
        using reference_this_type = ref<this_type>;
        using const_reference_this_type = cref<this_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 27.08.2021
         */
        constexpr ManageGuard() noexcept :
            _ptr(nullptr),
            _owned_flags(ResourceUsageFlag::eNone) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param [in,out] value_ The value.
         */
        [[nodiscard]] explicit ManageGuard(reference_type value_) :
            _ptr(_STD addressof(value_)),
            _owned_flags(ResourceUsageFlag::eNone) {
            acquire();
        }

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param [in,out] value_ The value.
         * @param          owned_flags_ The owned flags.
         */
        ManageGuard(reference_type value_, const ResourceUsageFlags owned_flags_) noexcept :
            _ptr(_STD addressof(value_)),
            _owned_flags(owned_flags_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 28.08.2021
         */
        ManageGuard(const_reference_this_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  other_ The other.
         */
        [[nodiscard]] ManageGuard(mref<this_type> other_) noexcept :
            _ptr(_STD exchange(other_._ptr, nullptr)),
            _owned_flags(_STD exchange(other_._owned_flags, ResourceUsageFlag::eNone)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 28.08.2021
         */
        ~ManageGuard() noexcept {
            if (_owned_flags != ResourceUsageFlag::eNone) {
                release();
            }
        }

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @returns A shallow copy of this.
         */
        reference_this_type operator=(const_reference_this_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_this_type operator=(mref<this_type> other_) {
            if (this != _STD addressof(other_)) {
                if (_owned_flags != ResourceUsageFlag::eNone) {
                    release();
                }

                _ptr = _STD exchange(other_._ptr, nullptr);
                _owned_flags = _STD exchange(other_._owned_flags, ResourceUsageFlag::eNone);
            }

            return *this;
        }

    private:
        ptr<value_type> _ptr;
        ResourceUsageFlags _owned_flags;

    private:
        void swap(reference_this_type other_) noexcept {
            _STD swap(_ptr, other_._ptr);
            _STD swap(_owned_flags, other_._owned_flags);
        }

    public:
        [[nodiscard]] bool try_acquire() noexcept {
            return _ptr->try_acquire(this, _owned_flags);
        }

        void acquire(const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) noexcept {
            swap(_ptr->acquire(flags_));
        }

        reference_this_type acquire(reference_type value_, const ResourceUsageFlags flags_ = ResourceUsageFlag::eDefault) {
            swap(_STD forward<value_type>(value_.acquire(flags_)));
            return *this;
        }

        reference_this_type release() {
            _ptr->release(_owned_flags);
            _owned_flags = ResourceUsageFlag::eNone;
        }

    public:
        [[nodiscard]] bool owns() const noexcept {
            return _owned_flags != ResourceUsageFlag::eNone;
        }

        explicit operator bool() const noexcept {
            return _owned_flags != ResourceUsageFlag::eNone;
        }

        [[nodiscard]] const ResourceUsageFlags owned_flags() const noexcept {
            return _owned_flags;
        }

    public:
        [[nodiscard]] ptr<value_type> managed() const noexcept {
            return const_cast<ptr<value_type>>(_ptr);
        }

        [[nodiscard]] ptr<value_type> managed() noexcept {
            return _ptr;
        }

        [[nodiscard]] ptr<value_type> operator->() const noexcept {
            return const_cast<ptr<value_type>>(_ptr);
        }

        [[nodiscard]] ptr<value_type> operator->() noexcept {
            return _ptr;
        }

        [[nodiscard]] const_reference_type operator*() const noexcept {
            return *_ptr;
        }

        [[nodiscard]] reference_type operator*() {
            return *_ptr;
        }

    };

}
