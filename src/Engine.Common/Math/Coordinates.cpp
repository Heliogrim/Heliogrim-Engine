#include "Coordinates.hpp"

#include <cmath>

using namespace hg::math;
using namespace hg;

vec3 polar::from_cartesian(cref<vec3> cartesian_) noexcept {

    const auto r { cartesian_.length() };

    return vec3 {
        r,
        _STD acos(cartesian_.x / _STD sqrt(cartesian_.x * cartesian_.x + cartesian_.z * cartesian_.z)) * (
            cartesian_.z < 0 ? -1 : 1),
        _STD acos(cartesian_.y / r)
    };
}

vec3 polar::from_cartesian(mref<vec3> cartesian_) noexcept {

    const auto r { cartesian_.length() };

    return vec3 {
        r,
        _STD acos(cartesian_.x / _STD sqrt(cartesian_.x * cartesian_.x + cartesian_.z * cartesian_.z)) * (
            cartesian_.z < 0 ? -1 : 1),
        _STD acos(cartesian_.y / r)
    };
}

vec2 polar::from_cartesian(cref<vec2> cartesian_) noexcept {
    return vec2 {
        cartesian_.length(),
        _STD atan(cartesian_.y / cartesian_.x)
    };
}

vec2 polar::from_cartesian(mref<vec2> cartesian_) noexcept {
    return vec2 {
        cartesian_.length(),
        _STD atan(cartesian_.y / cartesian_.x)
    };
}

vec3 polar::to_cartesian(cref<vec3> polar_) noexcept {

    const auto xsl = polar_.x * _STD sin(polar_.z);

    return vec3 {
        xsl * _STD cos(polar_.y),
        polar_.x * _STD cos(polar_.z),
        xsl * _STD sin(polar_.y),
    };
}

vec3 polar::to_cartesian(mref<vec3> polar_) noexcept {

    const auto xsl = polar_.x * _STD sin(polar_.z);

    return vec3 {
        xsl * _STD cos(polar_.y),
        polar_.x * _STD cos(polar_.z),
        xsl * _STD sin(polar_.y),
    };
}

vec2 polar::to_cartesian(cref<vec2> polar_) noexcept {
    return vec2 {
        polar_.x * _STD cos(polar_.y),
        polar_.x * _STD sin(polar_.y)
    };
}

vec2 polar::to_cartesian(mref<vec2> polar_) noexcept {
    return vec2 {
        polar_.x * _STD cos(polar_.y),
        polar_.x * _STD sin(polar_.y)
    };
}
