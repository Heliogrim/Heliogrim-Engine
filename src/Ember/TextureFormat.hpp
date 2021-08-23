#pragma once
#include <Engine.GFX/TextureFormat.hpp>

namespace ember {

    using TextureFormat = engine::gfx::TextureFormat;

    /**
     * Query if 'format_' is depth format
     *
     * @author Julius
     * @date 07.01.2021
     *
     * @param  format_ The format to check
     *
     * @returns True if depth format, false if not.
     */
    [[nodiscard]] bool isDepthFormat(const TextureFormat& format_) noexcept;

    /**
     * Query if 'format_' is stencil format
     *
     * @author Julius
     * @date 07.01.2021
     *
     * @param  format_ The format to check
     *
     * @returns True if stencil format, false if not.
     */
    [[nodiscard]] bool isStencilFormat(const TextureFormat& format_) noexcept;

    /**
     * Query if 'format_' is depth stencil format
     *
     * @author Julius
     * @date 07.01.2021
     *
     * @param  format_ The format to check
     *
     * @returns True if depth stencil format, false if not.
     */
    [[nodiscard]] bool isDepthStencilFormat(const TextureFormat& format_) noexcept;
}
