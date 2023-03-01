#pragma once

#if !defined MATH_DEFAULT_SINGLE_PRECISION && !defined MATH_DEFAULT_DOUBLE_PRECISION
#define MATH_DEFAULT_SINGLE_PRECISION
#endif

#if defined MATH_DEFAULT_SINGLE_PRECISION && defined MATH_DEFAULT_DOUBLE_PRECISION
#undef MATH_DEFAULT_SINGLE_PRECISION
#endif

#ifdef MATH_DEFAULT_SINGLE_PRECISION
typedef float mathGenericType;
#endif

#ifdef MATH_DEFAULT_DOUBLE_PRECISION
typedef double mathGenericType;
#endif

namespace hg::math {
    template <typename T>
    constexpr T clamp(T val_, T min_, T max_) {
        return MIN(MAX(val_, min_), max_);
    }
}
