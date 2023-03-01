#pragma once

#include "../Wrapper.hpp"

#include "__const.hpp"
#include "MathDefaultDefine.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"

namespace hg::math {
    template <class From_, class To_>
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
        return mat4 { as<quaternion, mat3>(from_) };
    }
}
