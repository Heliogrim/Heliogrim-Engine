#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel {
    enum class TransferDataType: u8 {
        eUnknown,
        /**/
        eU8,
        eU16,
        eU32,
        eU64,
        eF16,
        eF32,
        /**/
        eU8Vec2,
        eU16Vec2,
        eU32Vec2,
        eU64Vec2,
        eF16Vec2,
        eF32Vec2,
        eU8Vec3,
        eU16Vec3,
        eU32Vec3,
        eU64Vec3,
        eF16Vec3,
        eF32Vec3,
        eU8Vec4,
        eU16Vec4,
        eU32Vec4,
        eU64Vec4,
        eF16Vec4,
        eF32Vec4,
        /**/
        eF16Mat3x3,
        eF32Mat3x3,
        eF16Mat4x4,
        eF32Mat4x4,
        /**/
        eConstant,
        eUniform,
        eStorage,
        eSampler
    };
}
