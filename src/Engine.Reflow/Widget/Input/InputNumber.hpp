#pragma once

#include "InputFloat.hpp"
#include "InputIntegral.hpp"

namespace ember::engine::reflow {

    template <typename ValueType_>
    struct TypedInputNumber {
        using type = void;
    };

    template <>
    struct TypedInputNumber<s8> {
        using type = InputIntegral;
    };

    template <>
    struct TypedInputNumber<s16> {
        using type = InputIntegral;
    };

    template <>
    struct TypedInputNumber<s32> {
        using type = InputIntegral;
    };

    template <>
    struct TypedInputNumber<s64> {
        using type = InputIntegral;
    };

    template <>
    struct TypedInputNumber<float> {
        using type = InputFloat;
    };

    template <>
    struct TypedInputNumber<double> {
        using type = InputFloat;
    };

    /**/

    template <typename ValueType_>
    using InputType = typename TypedInputNumber<ValueType_>::type;

}
