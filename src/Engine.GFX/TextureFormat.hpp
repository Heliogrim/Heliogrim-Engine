#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    enum class TextureFormat {
        eUndefined = 0x0,

        //
        eR8Unorm,
        eR8G8Unorm,
        eR8G8B8Unorm,
        eR8G8B8A8Unorm,
        eR8G8B8A8Srgb,

        eB8G8R8Unorm,
        eB8G8R8A8Unorm,

        eR16Unorm,
        eR16G16Unorm,
        eR16G16B16Unorm,
        eR16G16B16A16Unorm,

        eR32Unorm [[deprecated]],
        eR32G32Unorm,
        eR32G32B32Unorm,
        eR32G32B32A32Unorm,

        eR64Unorm [[deprecated]],
        eR64G64Unorm,
        eR64G64B64Unorm,
        eR64G64B64A64Unorm,

        //
        eR16Sfloat,
        eR32Sfloat,
        eR64Sfloat,

        eR16G16Sfloat,

        eR16G16B16Sfloat,
        eR32G32B32Sfloat,

        eR16G16B16A16Sfloat,
        eR32G32B32A32Sfloat,

        //
        eD16Unorm,
        eD32Sfloat,
        eS8Uint,
        eD16UnormS8Uint,
        eD24UnormS8Uint,
        eD32SfloatS8Uint,

        //
        eR16Uint,
        eR16G16Uint,
        eR32Uint
    };

    /**
     * Query if 'format_' is depth format
     *
     * @author Julius
     * @date 09.01.2021
     *
     * @param  format_ Describes the format to use.
     *
     * @returns True if depth format, false if not.
     */
    [[nodiscard]] bool isDepthFormat(cref<TextureFormat> format_) noexcept;

    /**
     * Query if 'format_' is stencil format
     *
     * @author Julius
     * @date 09.01.2021
     *
     * @param  format_ Describes the format to use.
     *
     * @returns True if stencil format, false if not.
     */
    [[nodiscard]] bool isStencilFormat(cref<TextureFormat> format_) noexcept;

    /**
     * Query if 'format_' is depth stencil format
     *
     * @author Julius
     * @date 09.01.2021
     *
     * @param  format_ Describes the format to use.
     *
     * @returns True if depth stencil format, false if not.
     */
    [[nodiscard]] bool isDepthStencilFormat(cref<TextureFormat> format_) noexcept;

    /**
     * Query the size of the data described by the format
     *
     * @author Julius
     * @date 25.07.2022
     *
     * @param   format_ The format to check
     *
     * @returns The amount of bytes used by a data block, otherwise -1
     */
    [[nodiscard]] s64 formatDataSize(const TextureFormat format_) noexcept;

    /**
     * Query the block dimensions for a certain format
     *
     * @details A `block` is a single pixel for uncompressed formats.
     *  Compressed formats will have a size of `block b :: x(b) * y(b) * z(b)`
     *
     * @author Julius
     * @date 05.03.2023
     *
     * @returns A vector representing the size of a block.
     */
    [[nodiscard]] math::uivec3 formatBlockSize(const TextureFormat format_) noexcept;
}
