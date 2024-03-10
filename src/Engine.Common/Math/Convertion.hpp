#pragma once

#include <utility>

#include "../Wrapper.hpp"

#include "__const.hpp"
#include "MathDefaultDefine.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Rotator.hpp"

namespace hg::math {
	template <class To_, class From_>
	inline static To_ as(cref<From_> from_) noexcept;

	template <>
	inline static mat3 as(cref<quaternion> from_) noexcept {

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

		mat[2][0] = mat3::value_type { 2 } * (xy + wy);
		mat[2][1] = mat3::value_type { 2 } * (yz - wx);
		mat[2][2] = mat3::value_type { 1 } - mat3::value_type { 2 } * (xx + yy);

		return mat;
	}

	template <>
	inline static mat4 as(cref<quaternion> from_) noexcept {
		return mat4 { as<mat3, quaternion>(from_) };
	}

	template <>
	inline static Rotator as(cref<quaternion> from_) noexcept {

		const auto& data = from_._quat;

		// @see http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
		constexpr static float singularity = 0.4999999F;

		/*
		const auto x2 = data.x * data.x;
		const auto y2 = data.y * data.y;
		const auto z2 = data.z * data.z;
		const auto w2 = data.w * data.w;
		 */

		const auto test = data.x * data.y + data.z * data.w;
		/*
		const auto unit = x2 + y2 + z2 + w2;
		 */

		float heading;
		float attitude;
		float bank;

		if (test > singularity/* * unit */) {

			heading = 2.F * std::atan2(data.x, data.w);
			attitude = math::pih_f /* North Pole :: Pitch == 90Deg */;
			bank = 0.F;

		} else if (test < -singularity/* * unit */) {

			heading = -2.F * std::atan2(data.x, data.w);
			attitude = -math::pih_f /* South Pole :: Pitch == -90Deg */;
			bank = 0.F;

		} else {

			const auto zp2 = data.z * data.z;

			heading = std::atan2(
				2.F * data.y * data.w - 2.F * data.x * data.z,
				1.F - 2.F * data.y * data.y - 2 * zp2
			);

			attitude = std::asin(2.F * test);

			bank = std::atan2(
				2.F * data.x * data.w - 2.F * data.y * data.z,
				1 - 2.F * data.x * data.x - 2.F * zp2
			);

		}

		return Rotator {
			bank,
			heading,
			attitude
		};
	}

	template <>
	static fquaternion as(cref<math::fmat3> from_) noexcept {

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
