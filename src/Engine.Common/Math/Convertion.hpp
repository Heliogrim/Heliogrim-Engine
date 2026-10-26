#pragma once

#include <algorithm>
#include <cmath>
#include <utility>

#include "Compat.inl"
#include "MathDefaultDefine.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Rotator.hpp"
#include "Vector.hpp"
#include "__const.hpp"
#include "../Wrapper.hpp"

namespace hg::math {
	template <class To_, class From_>
	To_ as(cref<From_> from_) noexcept;

	template <>
	inline mat3 as(cref<quaternion> from_) noexcept {

		mat3 mat {};

		const mat3::value_type xx { from_._quat.x * from_._quat.x };
		const mat3::value_type yy { from_._quat.y * from_._quat.y };
		const mat3::value_type zz { from_._quat.z * from_._quat.z };

		const mat3::value_type xz { from_._quat.x * from_._quat.z };
		const mat3::value_type xy { from_._quat.x * from_._quat.y };

		const mat3::value_type yz { from_._quat.y * from_._quat.z };

		const mat3::value_type wx { from_._quat.w * from_._quat.x };
		const mat3::value_type wy { from_._quat.w * from_._quat.y };
		const mat3::value_type wz { from_._quat.w * from_._quat.z };

		mat[0][0] = mat3::value_type { 1 } - mat3::value_type { 2 } * (yy + zz);
		mat[0][1] = mat3::value_type { 2 } * (xy + wz);
		mat[0][2] = mat3::value_type { 2 } * (xz - wy);

		mat[1][0] = mat3::value_type { 2 } * (xy - wz);
		mat[1][1] = mat3::value_type { 1 } - mat3::value_type { 2 } * (xx + zz);
		mat[1][2] = mat3::value_type { 2 } * (yz + wx);

		mat[2][0] = mat3::value_type { 2 } * (xz + wy);
		mat[2][1] = mat3::value_type { 2 } * (yz - wx);
		mat[2][2] = mat3::value_type { 1 } - mat3::value_type { 2 } * (xx + yy);

		return mat;
	}

	template <>
	inline mat4 as(cref<quaternion> from_) noexcept {
		return mat4 { as<mat3, quaternion>(from_) };
	}

	template <>
	inline Rotator as(cref<quaternion> from_) noexcept {

		const auto& data = from_._quat;

		/**
		 * Inverse of `quaternion::euler`, which composes `q = Rx(pitch) * Ry(yaw) * Rz(roll)`.
		 *
		 * The rotation matrix of that product has the following entries (s = sin, c = cos):
		 *	[0][2] =  s(yaw)
		 *	[1][2] = -s(pitch) * c(yaw)		[2][2] = c(pitch) * c(yaw)
		 *	[0][1] = -c(yaw) * s(roll)		[0][0] = c(yaw) * c(roll)
		 * which are read back from the quaternion components below.
		 */
		constexpr static float singularity = 0.999999F;

		const auto sinYaw = std::clamp(2.F * (data.x * data.z + data.w * data.y), -1.F, 1.F);

		float pitch;
		float roll;

		if (std::abs(sinYaw) < singularity) {

			pitch = math::atan2f(
				-2.F * (data.y * data.z - data.w * data.x),
				1.F - 2.F * (data.x * data.x + data.y * data.y)
			);

			roll = math::atan2f(
				-2.F * (data.x * data.y - data.w * data.z),
				1.F - 2.F * (data.y * data.y + data.z * data.z)
			);

		} else {

			/* Gimbal lock :: Yaw == +-90Deg, pitch and roll rotate around the same axis, so we fold all of it into pitch. */
			pitch = math::atan2f(
				2.F * (data.y * data.z + data.w * data.x),
				1.F - 2.F * (data.x * data.x + data.z * data.z)
			);
			roll = 0.F;
		}

		return Rotator {
			pitch,
			math::asinf(sinYaw),
			roll
		};
	}

	template <>
	inline fquaternion as(cref<math::fmat3> from_) noexcept {

		const auto fourXSquaredMinus1 = from_[0][0] - from_[1][1] - from_[2][2];
		const auto fourYSquaredMinus1 = from_[1][1] - from_[0][0] - from_[2][2];
		const auto fourZSquaredMinus1 = from_[2][2] - from_[0][0] - from_[1][1];
		const auto fourWSquaredMinus1 = from_[0][0] + from_[1][1] + from_[2][2];

		u8 biggestIndex = 0u;
		auto fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1u;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2u;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3u;
		}

		const auto biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.F) * .5F;
		const auto mult = .25F / biggestVal;

		switch (biggestIndex) {
			case 0u: return {
					biggestVal,
					(from_[1][2] - from_[2][1]) * mult,
					(from_[2][0] - from_[0][2]) * mult,
					(from_[0][1] - from_[1][0]) * mult
				};
			case 1u: return {
					(from_[1][2] - from_[2][1]) * mult,
					biggestVal,
					(from_[0][1] + from_[1][0]) * mult,
					(from_[2][0] + from_[0][2]) * mult
				};
			case 2u: return {
					(from_[2][0] - from_[0][2]) * mult,
					(from_[0][1] + from_[1][0]) * mult,
					biggestVal,
					(from_[1][2] + from_[2][1]) * mult
				};
			case 3u: return {
					(from_[0][1] - from_[1][0]) * mult,
					(from_[2][0] + from_[0][2]) * mult,
					(from_[1][2] + from_[2][1]) * mult,
					biggestVal
				};
			default: std::unreachable();
		}
	}
}
