#pragma once

#include "../Types.hpp"
#include "../String.hpp"
#include "../Wrapper.hpp"

namespace ember::hash {
    namespace {
        /**
         * Fnv-1a 86
         *
         * @author Julius
         * @date 06.09.2020
         *
         * @param  str_ The.
         * @param  count_ Number of.
         *
         * @returns A type_id.
         */
        constexpr u64 fnv1a_86(const char* str_, const _STD size_t count_) {
            return (
                (count_ ? fnv1a_86(str_, count_ - 1) : 2166136261u) ^ str_[count_]
            ) * 16777619u;
        }
    }

    constexpr u64 operator"" _fnv1a(const char* str_, const _STD size_t count_) {
        return fnv1a_86(str_, count_);
    }

    [[nodiscard]] constexpr u64 fnv1a(const char* str_, const _STD size_t count_) noexcept {
        return fnv1a_86(str_, count_);
    }

    [[nodiscard]] constexpr u64 fnv1a(cref<string_view> src_) noexcept {
        return fnv1a_86(src_.data(), src_.length());
    }

    [[nodiscard]] inline u64 fnv1a(cref<string> src_) noexcept {
        return fnv1a_86(src_.data(), src_.length());
    }
}
