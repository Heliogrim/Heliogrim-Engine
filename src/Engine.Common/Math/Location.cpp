#include "Location.hpp"

using namespace hg::math;
using namespace hg;

Location::Location(const value_type x_, const value_type y_, const value_type z_) noexcept :
	math_type(x_, y_, z_) {}

Location::Location(cref<math_type> location_) noexcept :
	math_type(std::move(location_)) {}

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

cref<fvec3> Location::into() const noexcept {
	return *this;
}
