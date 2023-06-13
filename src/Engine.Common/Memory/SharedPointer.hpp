#pragma once

#include <memory>
#include <Engine.Pedantic/Into/IntoImpl.hpp>

#include "../__macro.hpp"

namespace hg {
    template <typename To_, typename From_>
    struct into_impl<_STD shared_ptr<To_>, _STD shared_ptr<From_>> {
        [[nodiscard]] constexpr _STD shared_ptr<To_> operator()(_STD shared_ptr<From_>&& obj_) noexcept {
            return _STD shared_ptr<To_> { _STD move(obj_) };
        }
    };

    /**/

    template <typename To_, typename From_>
    struct into_impl<_STD shared_ptr<To_>, _STD unique_ptr<From_>> {
        [[nodiscard]] constexpr _STD shared_ptr<To_> operator()(_STD unique_ptr<From_>&& obj_) noexcept {
            return _STD shared_ptr<To_> { _STD move(obj_) };
        }
    };
}
