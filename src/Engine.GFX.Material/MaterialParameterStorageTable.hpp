#pragma once

#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>

#include "MaterialParameterStorage.hpp"

namespace hg::engine::gfx::material::table {
    /**/
    using Sampler = MaterialParameterStorage<smr<TextureResource>>;
    using Uniform = MaterialParameterStorage<smr<void>>;
    using Storage = MaterialParameterStorage<smr<void>>;
    using Constant = MaterialParameterStorage<smr<void>>;
    /**/
    using U8 = MaterialParameterStorage<u8>;
    using U16 = MaterialParameterStorage<u16>;
    using U32 = MaterialParameterStorage<u32>;
    using U64 = MaterialParameterStorage<u64>;
    using F16 = void;
    using F32 = MaterialParameterStorage<float>;
    /**/
    using U8Vec2 = MaterialParameterStorage<math::vec2_t<u8>>;
    using U16Vec2 = MaterialParameterStorage<math::vec2_t<u16>>;
    using U32Vec2 = MaterialParameterStorage<math::vec2_t<u32>>;
    using U64Vec2 = MaterialParameterStorage<math::vec2_t<u64>>;
    using F16Vec2 = void;
    using F32Vec2 = MaterialParameterStorage<math::vec3_t<float>>;
    /**/
    using U8Vec3 = MaterialParameterStorage<math::vec3_t<u8>>;
    using U16Vec3 = MaterialParameterStorage<math::vec3_t<u16>>;
    using U32Vec3 = MaterialParameterStorage<math::vec3_t<u32>>;
    using U64Vec3 = MaterialParameterStorage<math::vec3_t<u64>>;
    using F16Vec3 = void;
    using F32Vec3 = MaterialParameterStorage<math::vec3_t<float>>;
    /**/
    using U8Vec4 = MaterialParameterStorage<math::vec4_t<u8>>;
    using U16Vec4 = MaterialParameterStorage<math::vec4_t<u16>>;
    using U32Vec4 = MaterialParameterStorage<math::vec4_t<u32>>;
    using U64Vec4 = MaterialParameterStorage<math::vec4_t<u64>>;
    using F16Vec4 = void;
    using F32Vec4 = MaterialParameterStorage<math::vec4_t<float>>;
    /**/
    using F16Mat3x3 = void;
    using F32Mat3x3 = MaterialParameterStorage<math::matq3_t<float>>;
    using F16Mat4x4 = void;
    using F32Mat4x4 = MaterialParameterStorage<math::matq4_t<float>>;
    /**/
}
