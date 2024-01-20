#include "Rotator.hpp"
#include "__const.hpp"
#include "Matrix.hpp"
#include "Coordinates.hpp"
#include "Convertion.hpp"

using namespace hg::math;
using namespace hg;

Rotator::Rotator() noexcept = default;

Rotator::Rotator(const value_type pitch_, const value_type yaw_, const value_type roll_) noexcept :
    math_type(pitch_, yaw_, roll_) {}

Rotator::Rotator(mref<math::quaternion> quatern_) noexcept :
    math_type(math::as<math::Rotator>(_STD move(quatern_))) {}

Rotator::Rotator(mref<this_type> other_) noexcept :
    math_type(_STD move(other_)) {}

Rotator::Rotator(cref<this_type> other_) noexcept :
    math_type(other_) {}

ref<Rotator::this_type> Rotator::operator=(mref<this_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        math_type::x = _STD move(other_.x);
        math_type::y = _STD move(other_.y);
        math_type::z = _STD move(other_.z);
    }
    return *this;
}

ref<Rotator::this_type> Rotator::operator=(cref<this_type> other_) noexcept {
    if (this != _STD addressof(other_)) {
        math_type::x = other_.x;
        math_type::y = other_.y;
        math_type::z = other_.z;
    }
    return *this;
}

Rotator::value_type Rotator::pitch() const noexcept {
    return math_type::x;
}

Rotator::value_type Rotator::yaw() const noexcept {
    return math_type::y;
}

Rotator::value_type Rotator::roll() const noexcept {
    return math_type::z;
}

Rotator::math_type Rotator::inversed(cref<math_type> vector_) const noexcept {
    const auto inv = inversedRotator();
    return inv.rotated(vector_);
}

ref<Rotator::math_type> Rotator::inverse(ref<math_type> vector_) const noexcept {
    const auto inv = inversedRotator();
    return inv.rotate(vector_);
}

Rotator::math_type Rotator::rotated(cref<math_type> vector_) const noexcept {
    mat4 rotation = mat4::make_identity();
    rotation.rotate(pitch(), math::vec3_right);
    rotation.rotate(yaw(), math::vec3_up);
    rotation.rotate(roll(), math::vec3_forward);

    return rotation * vec4(vector_, 0.F);
}

ref<Rotator::math_type> Rotator::rotate(ref<math_type> vector_) const noexcept {
    mat4 rotation = mat4::make_identity();
    rotation.rotate(pitch(), math::vec3_right);
    rotation.rotate(yaw(), math::vec3_up);
    rotation.rotate(roll(), math::vec3_forward);

    vector_ = rotation * vec4(vector_, 0.F);
    return vector_;
}

Rotator::this_type Rotator::inversedRotator() const noexcept {
    Rotator tmp { *this };
    tmp.inverseRotator();
    return tmp;
}

ref<Rotator::this_type> Rotator::inverseRotator() noexcept {
    math_type::x = 0.F - math_type::x;
    math_type::y = 0.F - math_type::y;
    math_type::z = 0.F - math_type::z;
    return *this;
}

Rotator::value_type Rotator::normalizeAxis(float angle_) const noexcept {
    const auto factor = (angle_ + pi_f/*180.F*/) / pi2_f/*360.F*/;
    return angle_ - (_STD floorf(factor) * pi2_f/*360.F*/);
}

Rotator::this_type Rotator::normalizedRotator() const noexcept {
    return Rotator {
        normalizeAxis(math_type::x),
        normalizeAxis(math_type::y),
        normalizeAxis(math_type::z)
    };
}

ref<Rotator::this_type> Rotator::normalizeRotator() noexcept {
    math_type::x = normalizeAxis(math_type::x);
    math_type::y = normalizeAxis(math_type::y);
    math_type::z = normalizeAxis(math_type::z);
    return *this;
}

Rotator::value_type Rotator::denormalizeAxis(float angle_) const noexcept {
    const auto factor = angle_ / pi2_f /*360.F*/;
    return angle_ - (_STD floorf(factor) * pi2_f/*360.F*/);
}

Rotator::this_type Rotator::denormalizedRotator() const noexcept {
    return Rotator {
        denormalizeAxis(math_type::x),
        denormalizeAxis(math_type::y),
        denormalizeAxis(math_type::z)
    };
}

ref<Rotator::this_type> Rotator::denormalizeRotator() noexcept {
    math_type::x = denormalizeAxis(math_type::x);
    math_type::y = denormalizeAxis(math_type::y);
    math_type::z = denormalizeAxis(math_type::z);
    return *this;
}

Rotator::this_type Rotator::combine(cref<this_type> left_, cref<this_type> right_) noexcept {
    return Rotator {
        left_.quaternion() * right_.quaternion()
    };
}

Rotator::this_type Rotator::diff(cref<this_type> from_, cref<this_type> to_) noexcept {

    auto prev = from_.quaternion();
    prev._quat = glm::inverse(prev._quat);

    return Rotator {
        to_.quaternion() * prev
    };
}

Rotator::this_type Rotator::outerToInner(cref<this_type> subject_, cref<this_type> outer_) noexcept {

    auto prev = outer_.quaternion();
    prev._quat = glm::inverse(prev._quat);

    return math::as<Rotator>(prev * subject_.quaternion());
}

Rotator::this_type Rotator::innerToOuter(cref<this_type> subject_, cref<this_type> outer_) noexcept {
    return math::as<Rotator>(outer_.quaternion() * subject_.quaternion());
}

Rotator::this_type Rotator::operator-(cref<this_type> other_) const noexcept {
    return Rotator {
        math_type::x - other_.x,
        math_type::y - other_.y,
        math_type::z - other_.z
    };
}

Rotator::this_type Rotator::operator+(cref<this_type> other_) const noexcept {
    return Rotator {
        math_type::x + other_.x,
        math_type::y + other_.y,
        math_type::z + other_.z
    };
}

Rotator::this_type Rotator::operator*(value_type scale_) const noexcept {
    return Rotator {
        math_type::x * scale_,
        math_type::y * scale_,
        math_type::z * scale_
    };
}

ref<Rotator::this_type> Rotator::operator-=(cref<this_type> other_) noexcept {
    math_type::x -= other_.x;
    math_type::y -= other_.y;
    math_type::z -= other_.z;
    return *this;
}

ref<Rotator::this_type> Rotator::operator+=(cref<this_type> other_) noexcept {
    math_type::x += other_.x;
    math_type::y += other_.y;
    math_type::z += other_.z;
    return *this;
}

ref<Rotator::this_type> Rotator::operator*=(value_type scale_) noexcept {
    math_type::x *= scale_;
    math_type::y *= scale_;
    math_type::z *= scale_;
    return *this;
}

bool Rotator::operator==(cref<this_type> other_) const noexcept {
    return math_type::operator==(other_);
}

bool Rotator::operator!=(cref<this_type> other_) const noexcept {
    return math_type::operator!=(other_);
}

fvec3 Rotator::euler() const noexcept {
    return fvec3 { math_type::x, math_type::y, math_type::z };
}

Rotator Rotator::fromEuler(cref<fvec3> euler_) noexcept {
    return Rotator { quaternion::euler(euler_) };
}

Rotator Rotator::fromEulerRaw(cref<fvec3> euler_) noexcept {
    return Rotator { euler_.x, euler_.y, euler_.x };
}

quaternion Rotator::quaternion() const noexcept {
    return quaternion::euler(*this);
}
