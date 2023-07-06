#pragma once

#include <concepts>
#include <Engine.Common/Make.hpp>

namespace hg {
    template <typename Type_>
    struct clone_impl;

    template <typename Type_>
    concept is_clonable = _STD false_type::value;

    /*
    [[nodiscard]] auto clone(const auto& val_) {
        return val_;
    }
    */

    template <typename Type_>
    [[nodiscard]] constexpr Type_ clone(const Type_& val_) {
        return clone_impl<Type_> {}(val_);
    }

    template <typename Type_> requires _STD is_copy_constructible_v<Type_>
    struct clone_impl<uptr<Type_>> {
        [[nodiscard]] constexpr uptr<Type_> operator()(const uptr<Type_>& val_) const {
            return make_uptr<Type_>(static_cast<_STD add_const_t<decltype(*val_)>>(*val_));
        }
    };

    template <typename Type_> requires _STD is_copy_constructible_v<Type_>
    struct clone_impl<Type_> {
        [[nodiscard]] constexpr Type_ operator()(const Type_& val_) const noexcept {
            return Type_(val_);
        }
    };
}
