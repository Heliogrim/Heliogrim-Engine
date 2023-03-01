#pragma once

#include <concepts>

namespace hg::math {
    /**
     * A fast ceiling 2.
     *
     * @author Julius
     * @date 27.08.2021
     *
     * @tparam Type_ Type of the type.
     */
    template <_STD integral Type_>
    struct fast_ceil2 {
        using value_type = Type_;

        constexpr fast_ceil2() noexcept = default;

        [[nodiscard]] static constexpr bool ip2(const value_type value_) noexcept {
            return value_ != 0 && (value_ & (value_ - 1u)) == 0;
        }

        [[nodiscard]] constexpr value_type operator()(value_type value_) noexcept {
            if (ip2(value_))
                return value_;

            if (value_ == 0)
                return 1;

            --value_;
            for (value_type i = 1; i < sizeof(value_type) * CHAR_BIT; i <<= 1) {
                value_ |= value_ >> i;
            }

            return value_ + 1u;
        }
    };
}
