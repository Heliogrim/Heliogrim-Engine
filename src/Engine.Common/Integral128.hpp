#pragma once

#include <concepts>
#include <cmath>
#include <type_traits>

namespace hg {
    struct uint128_t {
        constexpr uint128_t() noexcept = default;

        constexpr uint128_t(const uint128_t& other_) noexcept = default;

        constexpr uint128_t(uint128_t&& other_) noexcept = default;

        template <std::integral HighType_, std::integral LowType_>
        uint128_t(const HighType_& high_, const LowType_& low_) :
            low(low_),
            high(high_) {}

        uint128_t& operator=(const uint128_t& other_) noexcept = default;

        uint128_t& operator=(uint128_t&& other_) noexcept = default;

        template <std::integral Type_>
        uint128_t& operator=(const Type_& other_) {

            if constexpr (std::is_signed_v<Type_>) {
                high = other_ < 0 ? -1 : 0;
                low = other_;

            } else {
                high = 0;
                low = other_;
            }

            return *this;
        }

        constexpr operator bool() const noexcept {
            return (bool)(low) || (bool)(high);
        }

        constexpr operator uint8_t() const noexcept {
            return (uint8_t)(low);
        }

        constexpr operator uint16_t() const noexcept {
            return (uint16_t)(low);
        }

        constexpr operator uint32_t() const noexcept {
            return (uint32_t)(low);
        }

        constexpr operator uint64_t() const noexcept {
            return (uint64_t)(low);
        }

    public:
        [[nodiscard]] bool operator!() const noexcept {
            return low == 0 && high == 0;
        }

        [[nodiscard]] bool operator==(const uint128_t&) const noexcept = default;

        [[nodiscard]] bool operator!=(const uint128_t&) const noexcept = default;

    public:
        #if IS_BIG_ENDIAN
        uint64_t high, low;
        #else
        uint64_t low, high;
        #endif
    };

    /*
    constexpr uint128_t operator"" _ui128(const char* bytes_, std::size_t length_) {
        // 0x?? || 0o?? || 0b?? || ??
        return uint128_t(bytes_);
    }
     */
}

namespace std {
    template <>
    struct is_integral<::hg::uint128_t> : std::true_type {};

    template <>
    struct is_unsigned<::hg::uint128_t> : std::true_type {};
}
