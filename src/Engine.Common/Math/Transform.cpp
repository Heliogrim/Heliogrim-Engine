#include "Transform.hpp"

using namespace hg::math;
using namespace hg;

Transform::Transform() noexcept :
    _location(),
    _rotator(),
    _scale(1.F) {}

Transform::Transform(mref<Location> location_, mref<Rotator> rotator_, mref<fvec3> scale_) noexcept :
    _location(_STD move(location_)),
    _rotator(_STD move(rotator_)),
    _scale(_STD move(scale_)) {}

ref<Transform> Transform::operator=(mref<Transform> other_) noexcept {
    if (this != _STD addressof(other_)) {
        _location = _STD move(other_._location);
        _rotator = _STD move(other_._rotator);
        _scale = _STD move(other_._scale);
    }
    return *this;
}

ref<Transform> Transform::operator=(cref<Transform> other_) noexcept {
    if (this != _STD addressof(other_)) {
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
    _location = _STD move(location_);
    return *this;
}

cref<Rotator> Transform::rotator() const noexcept {
    return _rotator;
}

ref<Rotator> Transform::rotator() noexcept {
    return _rotator;
}

ref<Transform::this_type> Transform::setRotator(mref<Rotator> rotator_) noexcept {
    _rotator = _STD move(rotator_);
    return *this;
}

cref<fvec3> Transform::scale() const noexcept {
    return _scale;
}

ref<fvec3> Transform::scale() noexcept {
    return _scale;
}

ref<Transform::this_type> Transform::setScale(mref<fvec3> scale_) noexcept {
    _scale = _STD move(scale_);
    return *this;
}
