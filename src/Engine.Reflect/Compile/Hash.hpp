#pragma once

#include <Engine.Common/Meta/Concept.hpp>
#include "../CompileString.hpp"

namespace hg {
    template <typename Type_>
    struct CompileHash;

    /**/

    template <typename Type_> requires std::is_integral_v<Type_>
    struct CompileHash<Type_> {
        [[nodiscard]] constexpr size_t operator()(const Type_& value_) const noexcept {
            // TODO: Make compile-time hash for integral values
            return value_;
        }
    };

    template <>
    struct CompileHash<CompileString> {
        [[nodiscard]] constexpr size_t operator()(const CompileString& value_) const noexcept {
            return value_.hash();
        }
    };

    // static_assert(std::_Always_false<const char*>, "Please use std::string_view instead.");

    template <>
    struct CompileHash<string_view> {
        [[nodiscard]] constexpr size_t operator()(const string_view& value_) const noexcept {
            return hash::fnv1a(value_);
        }
    };
}
