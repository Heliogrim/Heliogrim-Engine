#include "Bounding.hpp"

using namespace hg::math;
using namespace hg;

Bounding::Bounding() :
    _center(vec3 { 0.F }),
    _extent(vec3 { 0.F }),
    _extent_length(0.F) {}

Bounding::Bounding(cref<vec3> center_, cref<vec3> extent_) noexcept :
    _center(center_),
    _extent(extent_),
    _extent_length() {}

bool Bounding::intersectsSpherical(cref<vec3> position_) const noexcept {
    const auto distance = (_center - position_).sqrLength();
    return MAX(distance, -distance) <= _extent_length * _extent_length;
}

bool Bounding::intersectsSpherical(cref<vec3> position_, const float radius_) const noexcept {
    const auto distance = (_center - position_).sqrLength();
    return MAX(distance, -distance) <= (_extent_length * _extent_length) + (radius_ * radius_);
}

bool Bounding::intersectsSpherical(cref<Bounding> other_) const noexcept {
    return intersectsSpherical(other_._center, other_._extent_length);
}

bool Bounding::intersectsAaBb(cref<vec3> position_) const noexcept {
    const auto delta = _center - position_;
    return (
        (_extent.x <= delta.x && -_extent.x >= delta.x) &&
        (_extent.y <= delta.y && -_extent.y >= delta.y) &&
        (_extent.z <= delta.z && -_extent.z >= delta.z)
    );
}

bool Bounding::intersectsAaBb(cref<vec3> position_, cref<vec3> extent_) const noexcept {
    const auto self_max = _center + _extent;
    const auto self_min = _center - _extent;

    const auto other_max = position_ + extent_;
    const auto other_min = position_ - extent_;

    return (
        (self_min.x <= other_max.x && self_max.x >= other_min.x) &&
        (self_min.y <= other_max.y && self_max.y >= other_min.y) &&
        (self_min.z <= other_max.z && self_max.z >= other_min.z)
    );
}

bool Bounding::intersectsAaBb(cref<Bounding> other_) const noexcept {
    return intersectsAaBb(other_._center, other_._extent);
}

bool Bounding::containsSpherical(cref<vec3> position_, cref<vec3::value_type> radius_) const noexcept {
    const auto distance = (_center - position_).sqrLength();
    return (MAX(distance, -distance) + radius_ * radius_) <= _extent_length * _extent_length;
}

bool Bounding::containsAaBb(cref<vec3> position_, cref<vec3> extent_) const noexcept {
    const auto self_max = _center + _extent;
    const auto self_min = _center - _extent;

    const auto other_max = position_ + extent_;
    const auto other_min = position_ - extent_;

    return (
        (self_max.x >= other_max.x && self_min.x <= other_min.x) &&
        (self_max.y >= other_max.y && self_min.y <= other_min.y) &&
        (self_max.z >= other_max.z && self_min.z <= other_min.z)
    );
}

cref<vec3> Bounding::center() const noexcept {
    return _center;
}

ref<vec3> Bounding::center() noexcept {
    return _center;
}

void Bounding::restoreLength() {
    _extent_length = _extent.length();
}

cref<vec3> Bounding::extent() const noexcept {
    return _extent;
}

cref<vec3::value_type> Bounding::extentLength() const noexcept {
    return _extent_length;
}

void Bounding::setExtent(cref<vec3> extent_) {
    _extent = extent_;
    restoreLength();
}

void Bounding::setExtent(cref<vec3::value_type> a_, cref<vec3::value_type> b_) {
    _extent.x = a_;
    _extent.y = b_;
    _extent.z = static_cast<vec3::value_type>(0);
    restoreLength();
}

void Bounding::setExtent(cref<vec3::value_type> x_, cref<vec3::value_type> y_, cref<vec3::value_type> z_) {
    _extent.x = x_;
    _extent.y = y_;
    _extent.z = z_;
    restoreLength();
}

void Bounding::applyExtentScale(cref<vec2::value_type> scale_) {
    _extent *= scale_;
    _extent_length *= scale_;
}
