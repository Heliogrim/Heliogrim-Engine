#pragma once

#include <concepts>
#include <type_traits>

#include "../__macro.hpp"

/** . */
namespace hg::hash {
    namespace {
        constexpr size_t magic_number = 0x9e3779b9;
    }

    /**
     * Hash combine
     *
     * @author Julius
     * @date 30.01.2020
     *
     * @param [in,out] seed_ The seed.
     * @param 		   hash_ The hash.
     */
    inline void hash_combine(size_t& seed_, size_t hash_) {
        hash_ += magic_number + (seed_ << 6) + (seed_ >> 2);
        seed_ ^= hash_;
    }

    template <std::integral SeedType_, typename ValueType_> requires std::integral<_STD remove_cvref_t<ValueType_>>
    FORCE_INLINE void hashCombineImpl(SeedType_& seed_, ValueType_&& value_) {

        constexpr static auto rotl = 6 * (sizeof(ValueType_) / sizeof(size_t));
        constexpr static auto rotr = 2 * (sizeof(ValueType_) / sizeof(size_t));

        seed_ ^= value_ + magic_number + (seed_ << rotl) + (seed_ >> rotr);
    }

    template <std::integral SeedType_, typename FirstType_, typename... Rest_> requires
        std::integral<_STD remove_cvref_t<FirstType_>> &&
        std::conjunction_v<std::is_integral<_STD remove_cvref_t<Rest_>>...>
    FORCE_INLINE void hashCombine(SeedType_& seed_, FirstType_&& first_, Rest_&&... rest_) {
        hashCombineImpl(seed_, first_);
        ((hashCombineImpl(seed_, rest_)), ...);
    }
}
