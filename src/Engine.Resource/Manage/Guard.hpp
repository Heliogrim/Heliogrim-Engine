#pragma once

#include <mutex>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::resource {

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
            _owns(false) {}

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
            _owns(false) {
            acquire();
        }

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param [in,out] value_ The value.
         * @param          adopt_lock_t The adopt lock t.
         */
        ManageGuard(reference_type value_, _STD adopt_lock_t) noexcept :
            _ptr(_STD addressof(value_)),
            _owns(true) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 28.08.2021
         *
         * @param [in,out] value_ The value.
         * @param          defer_lock_t The defer lock t.
         */
        [[nodiscard]] ManageGuard(reference_type value_, _STD defer_lock_t) noexcept :
            _ptr(_STD addressof(value_)),
            _owns(false) {}

    private:
        [[nodiscard]] ManageGuard(reference_type value_, _STD try_to_lock_t) noexcept :
            _ptr(_STD addressof(value_)),
            _owns(false) {
            try_acquire();
        }

    public:
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
            _owns(_STD exchange(other_._owns, false)) {}

        ~ManageGuard() noexcept {
            release();
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
                if (_owns) {
                    release();
                }

                _ptr = _STD exchange(other_._ptr, nullptr);
                _owns = _STD exchange(other_._owns, false);
            }

            return *this;
        }

    private:
        ptr<value_type> _ptr;
        bool _owns;

    private:
        void swap(reference_this_type other_) noexcept {
            _STD swap(_ptr, other_._ptr);
            _STD swap(_owns, other_._owns);
        }

    public:
        [[nodiscard]] bool try_acquire() noexcept {
            _owns = _ptr->try_acquire(this);
        }

        void acquire() noexcept {
            swap(_ptr->acquire());
        }

        reference_this_type acquire(reference_type value_) {
            swap(_STD forward<value_type>(value_.acquire()));
            return *this;
        }

        reference_this_type release() {
            _ptr->release();
            _owns = false;
        }

    public:
        [[nodiscard]] bool owns() const noexcept {
            return _owns;
        }

        explicit operator bool() const noexcept {
            return _owns;
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
