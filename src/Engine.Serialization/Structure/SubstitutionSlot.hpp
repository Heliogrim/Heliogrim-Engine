#pragma once

#include <concepts>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

#include "IntegralSlot.hpp"
#include "StringSlot.hpp"

namespace hg::engine::serialization {
    template <typename ValueType_>
    struct SubstitutionSlot;

    template <typename StringType_> requires std::is_same_v<StringType_, string>
    struct SubstitutionSlot<StringType_> {
        using type = StringSlot;
    };

    template <typename ValueType_> requires std::is_integral_v<ValueType_>
    struct SubstitutionSlot<ValueType_> {
        using type = IntegralSlot<ValueType_>;
    };
}
