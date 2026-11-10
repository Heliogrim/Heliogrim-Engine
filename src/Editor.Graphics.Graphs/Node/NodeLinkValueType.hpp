#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::editor::gfx::graph {
    enum class NodeLinkValueType {
        eUnknown = 0x0,

        /**/

        s8 = 0x05,
        u8 = 0x06,
        s16 = 0x07,
        u16 = 0x08,
        s32 = 0x09,
        u32 = 0x0A,
        s64 = 0x0B,
        u64 = 0x0C,

        /**/

        f16 = 0x10,
        f32 = 0x11,
        f64 = 0x12,

        /**/

        i8vec2,
        i16vec2,
        i32vec2,
        i64vec2,

        f16vec2,
        f32vec2,
        f64vec2,

        /**/

        i8vec3,
        i16vec3,
        i32vec3,
        i64vec3,

        f16vec3,
        f32vec3,
        f64vec3,

        /**/

        i8vec4,
        i16vec4,
        i32vec4,
        i64vec4,

        f16vec4,
        f32vec4,
        f64vec4,

        /**/

        f16mat2x2,
        f32mat2x2,
        f64mat2x2,

        /**/

        f16mat3x3,
        f32mat3x3,
        f64mat3x3,

        /**/

        f16mat4x4,
        f32mat4x4,
        f64mat4x4,

        /**/

        color
    };
}
