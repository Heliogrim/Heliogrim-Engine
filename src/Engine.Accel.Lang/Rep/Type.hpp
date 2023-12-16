#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel::lang {
    enum class TypeCategory : u8 {
        eScalar = 0x1,
        eTexture = 0x2,
        eObject = 0x3
    };

    enum class ScalarType : u8 {
        eU8 = 0x01,
        eU16 = 0x02,
        eU32 = 0x03,
        eU64 = 0x04,
        eS8 = 0x08,
        eS16 = 0x09,
        eS32 = 0x0A,
        eS64 = 0x0B,
        eF16 = 0x0F,
        eF32 = 0x10,
        eF64 = 0x11,
        /**/
        // Warning: Unsure
        /**/
        eU8Vec2 = 0xA1,
        eU8Vec3 = 0xA2,
        eU8Vec4 = 0xA3,
        eF32Vec3 = 0xA4,
        eF32Vec4 = 0xA5,
    };

    enum class TextureType : u8 {
        eTexture1d = 0x1,
        eTexture1dArray = 0x2,
        eTexture2d = 0x3,
        eTexture2dArray = 0x4,
        eTexture3d = 0x5,
        eTexture3dArray = 0x6,
        eTextureCube = 0x10
    };

    enum class ObjectType : u8 {
        eUnknown = 0x0
    };

    struct Type {
        TypeCategory category;

        union {
            u8 typeData;
            ScalarType scalarType;
            TextureType textureType;
            ObjectType objectType;
        };

        [[nodiscard]] constexpr bool operator==(cref<Type> other_) const noexcept {
            return category == other_.category && typeData == other_.typeData;
        }
    };
}
