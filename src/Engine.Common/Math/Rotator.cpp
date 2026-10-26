#include "Rotator.hpp"

#include <algorithm>
#include <cmath>

#include "Compat.inl"
#include "Convertion.hpp"
#include "Coordinates.hpp"
#include "Matrix.hpp"
#include "__const.hpp"

using namespace hg::math;
using namespace hg;

Rotator::Rotator(const value_type pitch_, const value_type yaw_, const value_type roll_) noexcept :
	math_type(pitch_, yaw_, roll_) {}

Rotator::Rotator(mref<math::quaternion> quatern_) noexcept :
	math_type(math::as<math::Rotator>(std::move(quatern_))) {}

Rotator::Rotator(mref<this_type> other_) noexcept :
	math_type(std::move(other_)) {}

Rotator::Rotator(cref<this_type> other_) noexcept :
	math_type(other_) {}

ref<Rotator::this_type> Rotator::operator=(mref<this_type> other_) noexcept {
	if (this != std::addressof(other_)) {
		math_type::x = std::move(other_.x);
		math_type::y = std::move(other_.y);
		math_type::z = std::move(other_.z);
	}
	return *this;
}

ref<Rotator::this_type> Rotator::operator=(cref<this_type> other_) noexcept {
	if (this != std::addressof(other_)) {
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

/**
 * Closed form of `Rx(pitch) * Ry(yaw) * Rz(roll)` with X right, Y up and Z forward.
 *
 * A vector is transformed as `v' = M * v`. Six trigonometric calls and no matrix products, as this sits on the
 * hot path of every location and rotation calculation. The inverse of a rotation is its transpose, so the very
 * same coefficients serve `inversed` without any matrix inversion.
 */
std::array<Rotator::value_type, 9> Rotator::asRotationMatrix() const noexcept {

	const auto sp = std::sin(pitch());
	const auto cp = std::cos(pitch());
	const auto sy = std::sin(yaw());
	const auto cy = std::cos(yaw());
	const auto sr = std::sin(roll());
	const auto cr = std::cos(roll());

	return {
		cy * cr, -cy * sr, sy,
		cp * sr + sp * sy * cr, cp * cr - sp * sy * sr, -sp * cy,
		sp * sr - cp * sy * cr, sp * cr + cp * sy * sr, cp * cy
	};
}

Rotator::this_type Rotator::fromRotationMatrix(cref<std::array<value_type, 9>> matrix_) noexcept {

	constexpr auto gimbalLimit = 0.999999F;

	/* The entry [0][2] equals sin(yaw), see `asRotationMatrix`. */
	const auto sinYaw = std::clamp(matrix_[2], -1.F, 1.F);
	const auto yaw = std::asin(sinYaw);

	if (std::abs(sinYaw) < gimbalLimit) {
		return Rotator {
			std::atan2(-matrix_[5], matrix_[8]),
			yaw,
			std::atan2(-matrix_[1], matrix_[0])
		};
	}

	/* Gimbal lock :: Yaw == +-90Deg, pitch and roll rotate around the same axis, so we fold all of it into pitch. */
	return Rotator { std::atan2(matrix_[7], matrix_[4]), yaw, 0.F };
}

Rotator::math_type Rotator::inversed(cref<math_type> vector_) const noexcept {
	const auto m = asRotationMatrix();
	return math_type {
		m[0] * vector_.x + m[3] * vector_.y + m[6] * vector_.z,
		m[1] * vector_.x + m[4] * vector_.y + m[7] * vector_.z,
		m[2] * vector_.x + m[5] * vector_.y + m[8] * vector_.z
	};
}

ref<Rotator::math_type> Rotator::inverse(ref<math_type> vector_) const noexcept {
	vector_ = inversed(vector_);
	return vector_;
}

Rotator::math_type Rotator::rotated(cref<math_type> vector_) const noexcept {
	const auto m = asRotationMatrix();
	return math_type {
		m[0] * vector_.x + m[1] * vector_.y + m[2] * vector_.z,
		m[3] * vector_.x + m[4] * vector_.y + m[5] * vector_.z,
		m[6] * vector_.x + m[7] * vector_.y + m[8] * vector_.z
	};
}

ref<Rotator::math_type> Rotator::rotate(ref<math_type> vector_) const noexcept {
	vector_ = rotated(vector_);
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
	return angle_ - (math::floorf(factor) * pi2_f/*360.F*/);
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
	return angle_ - (math::floorf(factor) * pi2_f/*360.F*/);
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
		left_.into() * right_.into()
	};
}

Rotator::this_type Rotator::diff(cref<this_type> from_, cref<this_type> to_) noexcept {

	auto prev = from_.into();
	prev._quat = glm::inverse(prev._quat);

	return Rotator {
		to_.into() * prev
	};
}

Rotator::this_type Rotator::outerToInner(cref<this_type> subject_, cref<this_type> outer_) noexcept {

	auto prev = outer_.into();
	prev._quat = glm::inverse(prev._quat);

	return math::as<Rotator>(prev * subject_.into());
}

Rotator::this_type Rotator::innerToOuter(cref<this_type> subject_, cref<this_type> outer_) noexcept {
	return math::as<Rotator>(outer_.into() * subject_.into());
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
	return Rotator { euler_.x, euler_.y, euler_.z };
}

quaternion Rotator::into() const noexcept {
	return quaternion::euler(*this);
}
