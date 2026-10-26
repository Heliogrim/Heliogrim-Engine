#include "Transform.hpp"

#include <Engine.Common/Move.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Convertion.hpp"

using namespace hg::math;
using namespace hg;

Transform::Transform(mref<Location> location_, mref<Rotator> rotator_, mref<scale_type> scale_) noexcept :
	_location(::hg::move(location_)),
	_rotator(::hg::move(rotator_)),
	_scale(::hg::move(scale_)) {}

ref<Transform> Transform::operator=(mref<Transform> other_) noexcept {
	if (this != std::addressof(other_)) {
		_location = ::hg::move(other_._location);
		_rotator = ::hg::move(other_._rotator);
		_scale = ::hg::move(other_._scale);
	}
	return *this;
}

ref<Transform> Transform::operator=(cref<Transform> other_) noexcept {
	if (this != std::addressof(other_)) {
		_location = other_._location;
		_rotator = other_._rotator;
		_scale = other_._scale;
	}
	return *this;
}

cref<Location> Transform::location() const noexcept {
	return _location;
}

ref<Location> Transform::location() noexcept {
	return _location;
}

ref<Transform::this_type> Transform::setLocation(mref<Location> location_) noexcept {
	_location = std::move(location_);
	return *this;
}

cref<Rotator> Transform::rotator() const noexcept {
	return _rotator;
}

ref<Rotator> Transform::rotator() noexcept {
	return _rotator;
}

ref<Transform::this_type> Transform::setRotator(mref<Rotator> rotator_) noexcept {
	_rotator = std::move(rotator_);
	return *this;
}

cref<Transform::scale_type> Transform::scale() const noexcept {
	return _scale;
}

ref<Transform::scale_type> Transform::scale() noexcept {
	return _scale;
}

ref<Transform::this_type> Transform::setScale(mref<scale_type> scale_) noexcept {
	_scale = std::move(scale_);
	return *this;
}

ref<Transform::this_type> Transform::setLocationAndRotator(mref<Location> location_, mref<Rotator> rotator_) noexcept {
	_location = ::hg::move(location_);
	_rotator = ::hg::move(rotator_);
	return *this;
}

mat4 Transform::asMatrix() const noexcept {

	// Equals `translate * rotate * scale`: the scale stretches the rotation's columns, the location is the last column.
	const auto rotation = _rotator.asRotationMatrix();

	auto result = mat4::make_identity();
	for (auto row = 0uLL; row < 3uLL; ++row) {
		result[0][row] = rotation[row * 3uLL + 0uLL] * _scale[0];
		result[1][row] = rotation[row * 3uLL + 1uLL] * _scale[1];
		result[2][row] = rotation[row * 3uLL + 2uLL] * _scale[2];
	}

	const auto& location = _location.into();
	result[3][0] = location.x;
	result[3][1] = location.y;
	result[3][2] = location.z;

	return result;
}

mat4 math::operator*(ref<const Transform> outer_, ref<const Transform> inner_) noexcept {
	return outer_.asMatrix() * inner_.asMatrix();
}

mat4 math::operator*(ref<const mat4> outer_, ref<const Transform> inner_) noexcept {
	return outer_ * inner_.asMatrix();
}
