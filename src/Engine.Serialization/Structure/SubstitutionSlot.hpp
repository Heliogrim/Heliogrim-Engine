#pragma once

#include <concepts>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

#include "IntegralScopedSlot.hpp"
#include "StringScopedSlot.hpp"

namespace ember::engine::serialization {
    template <typename ValueType_>
    struct SubstitutionSlot;

    template <typename StringType_> requires _STD is_same_v<StringType_, string>
    struct SubstitutionSlot<StringType_> {
        using type = StringScopedSlot;
    };

    template <typename ValueType_> requires _STD is_integral_v<ValueType_>
    struct SubstitutionSlot<ValueType_> {
        using type = IntegralScopedSlot<ValueType_>;
    };
}
