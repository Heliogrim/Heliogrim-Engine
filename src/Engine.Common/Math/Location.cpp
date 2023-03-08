#include "Location.hpp"

using namespace hg::math;
using namespace hg;

Location::Location() noexcept = default;

Location::Location(const value_type x_, const value_type y_, const value_type z_) noexcept :
    math_type(x_, y_, z_) {}

Location::Location(cref<math_type> location_) noexcept :
    math_type(location_) {}

Location::Location(mref<Location> other_) noexcept :
    math_type(_STD move(other_)) {}

Location::Location(cref<Location> other_) noexcept = default;

ref<Location::this_type> Location::operator=(mref<this_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        math_type::x = _STD move(other_.x);
        math_type::y = _STD move(other_.y);
        math_type::z = _STD move(other_.z);
    }
    return *this;
}

ref<Location::this_type> Location::operator=(cref<this_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        math_type::x = other_.x;
        math_type::y = other_.y;
        math_type::z = other_.z;
    }
    return *this;
}

void Location::setX(const value_type x_) noexcept {
    math_type::x = x_;
}

void Location::setY(const value_type y_) noexcept {
    math_type::y = y_;
}

void Location::setZ(const value_type z_) noexcept {
    math_type::z = z_;
}

bool Location::operator==(cref<this_type> other_) const noexcept {
    return math_type::operator==(other_);
}

bool Location::operator!=(cref<this_type> other_) const noexcept {
    return math_type::operator!=(other_);
}

Location::operator vec3_t<float>() const noexcept {
    return *this;
}
