#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Reflect/EmberReflect.hpp>

namespace ember::engine::serialization {

    class DataLayoutReflect {
    public:
        using this_type = DataLayoutReflect;

    public:
        DataLayoutReflect();

        DataLayoutReflect(cref<this_type> other_) noexcept = default;

        DataLayoutReflect(mref<this_type> other_) noexcept = default;

        ~DataLayoutReflect() noexcept;

    private:
        unary_fnc<ptr<void>, const ptr<void>> _ipc;
        unary_fnc<void, const ptr<void>> _ipd;

        // TODO: check whether we want to store more helper functions like `indirect resize` or `indirect shrink_to_fit` for container optimization
        // binary_fnc<void, const ptr<void>, const u64> _pre; // `self->resize(const u64 count_)`
        // unary_fnc<void, const ptr<void>> _post; // `self->shrink_to_fit()` | `self->clear()`

    public:
        void constructInPlace(const ptr<void> dst_);

        void destroyInPlace(const ptr<void> dst_);

    private:
        template <typename Type_>
        static ptr<void> sipch(const ptr<void> dst_) {
            return new(dst_) Type_;
        }

        template <IsEmberObject Type_>
        static ptr<void> ripch(const ptr<void> dst_) {
            return EmberObject::createInPlace<Type_>(dst_);
        }

        template <typename Type_>
        static void sipdh(const ptr<void> dst_) {
            static_cast<const ptr<Type_>>(dst_)->~Type_();
        }

        template <IsEmberObject Type_>
        static void ripdh(const ptr<void> dst_) {
            EmberObject::destroyInPlace<Type_>(static_cast<const ptr<Type_>>(dst_));
        }

    public:
        template <typename SubjectType_> requires _STD is_default_constructible_v<SubjectType_>
        void storeType() {

            if constexpr (IsEmberObject<SubjectType_>) {
                this_type::_ipc = this_type::ripch<SubjectType_>;
            } else {
                this_type::_ipc = this_type::sipch<SubjectType_>;
            }

            /**/

            if constexpr (IsEmberObject<SubjectType_>) {
                this_type::_ipd = this_type::ripdh<SubjectType_>;
            } else {
                this_type::_ipd = this_type::sipdh<SubjectType_>;
            }

            /**/

            // TODO: optimization functions

        }
    };

}
