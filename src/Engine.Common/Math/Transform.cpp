#include "Transform.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Convertion.hpp"

using namespace hg::math;
using namespace hg;

Transform::Transform(mref<Location> location_, mref<Rotator> rotator_, mref<fvec3> scale_) noexcept :
	_location(std::move(location_)),
	_rotator(std::move(rotator_)),
	_scale(std::move(scale_)) {}

ref<Transform> Transform::operator=(mref<Transform> other_) noexcept {
	if (this != std::addressof(other_)) {
		_location = std::move(other_._location);
		_rotator = std::move(other_._rotator);
		_scale = std::move(other_._scale);
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

cref<fvec3> Transform::scale() const noexcept {
	return _scale;
}

ref<fvec3> Transform::scale() noexcept {
	return _scale;
}

ref<Transform::this_type> Transform::setScale(mref<fvec3> scale_) noexcept {
	_scale = std::move(scale_);
	return *this;
}

mat4 Transform::asMatrix() const noexcept {

	const auto translate = mat4::make_identity().translate(_location.into());
	const auto rotate = math::as<mat4>(_rotator.into());
	const auto scale = mat4::make_identity().unchecked_scale(_scale);

	return translate * rotate * scale;
}

mat4 math::operator*(ref<const Transform> outer_, ref<const Transform> inner_) noexcept {
	return outer_.asMatrix() * inner_.asMatrix();
}

mat4 math::operator*(ref<const mat4> outer_, ref<const Transform> inner_) noexcept {
	return outer_ * inner_.asMatrix();
}
