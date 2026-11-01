#pragma once

#include <array>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel::lang {
    enum class TypeCategory : u8 {
        eScalar = 0x1,
        eTexture = 0x2,
        eObject = 0x3
    };

    /**
     * Packed numeric type code: scalars keep their fixed codes, vectors are `0x40 + element * 3 + (n - 2)`
     * (element = position in `scalarElements`), float matrices `0x80 + float * 9 + (cols - 2) * 3 + (rows - 2)`.
     * Use `vectorOf` / `matrixOf` / the accessors below instead of the raw values; nothing persists these codes.
     */
    namespace detail {
        [[nodiscard]] constexpr u8 vectorCode(u8 element_, u8 n_) noexcept {
            return static_cast<u8>(0x40 + element_ * 3 + (n_ - 2));
        }

        [[nodiscard]] constexpr u8 matrixCode(u8 float_, u8 cols_, u8 rows_) noexcept {
            return static_cast<u8>(0x80 + float_ * 9 + (cols_ - 2) * 3 + (rows_ - 2));
        }
    }

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
        /* vectors: element type, then component count */
        eU8Vec2 = detail::vectorCode(0, 2),
        eU8Vec3 = detail::vectorCode(0, 3),
        eU8Vec4 = detail::vectorCode(0, 4),
        eU16Vec2 = detail::vectorCode(1, 2),
        eU16Vec3 = detail::vectorCode(1, 3),
        eU16Vec4 = detail::vectorCode(1, 4),
        eU32Vec2 = detail::vectorCode(2, 2),
        eU32Vec3 = detail::vectorCode(2, 3),
        eU32Vec4 = detail::vectorCode(2, 4),
        eU64Vec2 = detail::vectorCode(3, 2),
        eU64Vec3 = detail::vectorCode(3, 3),
        eU64Vec4 = detail::vectorCode(3, 4),
        eS8Vec2 = detail::vectorCode(4, 2),
        eS8Vec3 = detail::vectorCode(4, 3),
        eS8Vec4 = detail::vectorCode(4, 4),
        eS16Vec2 = detail::vectorCode(5, 2),
        eS16Vec3 = detail::vectorCode(5, 3),
        eS16Vec4 = detail::vectorCode(5, 4),
        eS32Vec2 = detail::vectorCode(6, 2),
        eS32Vec3 = detail::vectorCode(6, 3),
        eS32Vec4 = detail::vectorCode(6, 4),
        eS64Vec2 = detail::vectorCode(7, 2),
        eS64Vec3 = detail::vectorCode(7, 3),
        eS64Vec4 = detail::vectorCode(7, 4),
        eF16Vec2 = detail::vectorCode(8, 2),
        eF16Vec3 = detail::vectorCode(8, 3),
        eF16Vec4 = detail::vectorCode(8, 4),
        eF32Vec2 = detail::vectorCode(9, 2),
        eF32Vec3 = detail::vectorCode(9, 3),
        eF32Vec4 = detail::vectorCode(9, 4),
        eF64Vec2 = detail::vectorCode(10, 2),
        eF64Vec3 = detail::vectorCode(10, 3),
        eF64Vec4 = detail::vectorCode(10, 4),
        /* float matrices: element type, then columns x rows (GLSL `matCxR`) */
        eF16Mat2x2 = detail::matrixCode(0, 2, 2),
        eF16Mat2x3 = detail::matrixCode(0, 2, 3),
        eF16Mat2x4 = detail::matrixCode(0, 2, 4),
        eF16Mat3x2 = detail::matrixCode(0, 3, 2),
        eF16Mat3x3 = detail::matrixCode(0, 3, 3),
        eF16Mat3x4 = detail::matrixCode(0, 3, 4),
        eF16Mat4x2 = detail::matrixCode(0, 4, 2),
        eF16Mat4x3 = detail::matrixCode(0, 4, 3),
        eF16Mat4x4 = detail::matrixCode(0, 4, 4),
        eF32Mat2x2 = detail::matrixCode(1, 2, 2),
        eF32Mat2x3 = detail::matrixCode(1, 2, 3),
        eF32Mat2x4 = detail::matrixCode(1, 2, 4),
        eF32Mat3x2 = detail::matrixCode(1, 3, 2),
        eF32Mat3x3 = detail::matrixCode(1, 3, 3),
        eF32Mat3x4 = detail::matrixCode(1, 3, 4),
        eF32Mat4x2 = detail::matrixCode(1, 4, 2),
        eF32Mat4x3 = detail::matrixCode(1, 4, 3),
        eF32Mat4x4 = detail::matrixCode(1, 4, 4),
        eF64Mat2x2 = detail::matrixCode(2, 2, 2),
        eF64Mat2x3 = detail::matrixCode(2, 2, 3),
        eF64Mat2x4 = detail::matrixCode(2, 2, 4),
        eF64Mat3x2 = detail::matrixCode(2, 3, 2),
        eF64Mat3x3 = detail::matrixCode(2, 3, 3),
        eF64Mat3x4 = detail::matrixCode(2, 3, 4),
        eF64Mat4x2 = detail::matrixCode(2, 4, 2),
        eF64Mat4x3 = detail::matrixCode(2, 4, 3),
        eF64Mat4x4 = detail::matrixCode(2, 4, 4),
    };

    /* the scalar element types, in the order the vector and matrix codes index them */
    inline constexpr std::array<ScalarType, 11> scalarElements {
        ScalarType::eU8, ScalarType::eU16, ScalarType::eU32, ScalarType::eU64,
        ScalarType::eS8, ScalarType::eS16, ScalarType::eS32, ScalarType::eS64,
        ScalarType::eF16, ScalarType::eF32, ScalarType::eF64
    };

    [[nodiscard]] constexpr bool isVector(const ScalarType type_) noexcept {
        const auto code = static_cast<u8>(type_);
        return code >= 0x40 && code < 0x40 + 11 * 3;
    }

    [[nodiscard]] constexpr bool isMatrix(const ScalarType type_) noexcept {
        const auto code = static_cast<u8>(type_);
        return code >= 0x80 && code < 0x80 + 3 * 9;
    }

    [[nodiscard]] constexpr bool isFloatScalar(const ScalarType type_) noexcept {
        return type_ == ScalarType::eF16 || type_ == ScalarType::eF32 || type_ == ScalarType::eF64;
    }

    /* the vector of `n_` (2..4) components of a scalar type; the scalar itself for 1; `scalar_` unchanged if invalid */
    [[nodiscard]] constexpr ScalarType vectorOf(const ScalarType scalar_, const u8 n_) noexcept {
        if (n_ == 1) {
            return scalar_;
        }
        for (u8 element = 0; element < scalarElements.size(); ++element) {
            if (scalarElements[element] == scalar_ && n_ >= 2 && n_ <= 4) {
                return static_cast<ScalarType>(detail::vectorCode(element, n_));
            }
        }
        return scalar_;
    }

    /* the float matrix of a float scalar type with `cols_` x `rows_` (each 2..4); `scalar_` unchanged if invalid */
    [[nodiscard]] constexpr ScalarType matrixOf(const ScalarType scalar_, const u8 cols_, const u8 rows_) noexcept {
        if (not isFloatScalar(scalar_) || cols_ < 2 || cols_ > 4 || rows_ < 2 || rows_ > 4) {
            return scalar_;
        }
        const auto floating = static_cast<u8>(scalar_ == ScalarType::eF16 ? 0 : scalar_ == ScalarType::eF32 ? 1 : 2);
        return static_cast<ScalarType>(detail::matrixCode(floating, cols_, rows_));
    }

    /* the element type of a vector or matrix; a scalar is its own element */
    [[nodiscard]] constexpr ScalarType elementOf(const ScalarType type_) noexcept {
        const auto code = static_cast<u8>(type_);
        if (isVector(type_)) {
            return scalarElements[(code - 0x40) / 3];
        }
        if (isMatrix(type_)) {
            return scalarElements[8 + (code - 0x80) / 9];
        }
        return type_;
    }

    /* components of a vector (2..4), columns of a matrix, 1 for a scalar */
    [[nodiscard]] constexpr u8 componentCount(const ScalarType type_) noexcept {
        const auto code = static_cast<u8>(type_);
        return static_cast<u8>(isVector(type_) ? (code - 0x40) % 3 + 2 : isMatrix(type_) ? (code - 0x80) % 9 / 3 + 2 : 1);
    }

    /* rows of a matrix, 1 otherwise */
    [[nodiscard]] constexpr u8 rowCount(const ScalarType type_) noexcept {
        return static_cast<u8>(isMatrix(type_) ? (static_cast<u8>(type_) - 0x80) % 3 + 2 : 1);
    }

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
        eUnknown = 0x0,
        eSampler = 0x1
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
