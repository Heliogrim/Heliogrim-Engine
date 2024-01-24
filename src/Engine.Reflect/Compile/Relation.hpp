#pragma once

#include <Engine.Common/__macro.hpp>
#include <memory>

namespace hg {
    template <typename Type_>
    struct CompileLess {
        [[nodiscard]] constexpr bool operator()(const Type_& left_, const Type_& right_) const noexcept {
            return left_ < right_;
        }
    };

    template <typename Type_>
    struct CompileSwap {
        constexpr void operator()(Type_& left_, Type_& right_) const noexcept {
            constexpr Type_ tmp { std::move(left_) };
            left_ = std::move(right_);
            right_ = std::move(tmp);
        }
    };
}
