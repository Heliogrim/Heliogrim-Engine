#pragma once

#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    class Viewport {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        Viewport() noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @param  width_ The width.
         * @param  height_ The height.
         * @param  minDepth_ (Optional) The minimum depth.
         * @param  maxDepth_ (Optional) The maximum depth.
         */
        Viewport(const u32 width_, const u32 height_, const float minDepth_ = 0.F, const float maxDepth_ = 1.F);

        /**
         * Destructor
         *
         * @author Julius
         * @date 01.12.2020
         */
        ~Viewport() noexcept = default;

    private:
        /**
         * Extent Width / Height
         */
        math::uivec2 _extent;

    public:
        /**
         * Gets the width
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 width() const noexcept;

        /**
         * Gets the height
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 height() const noexcept;

    private:
        /**
         * Extent Depth
         */
        math::fvec2 _depthExtent;

    public:
        /**
         * Minimum depth
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float minDepth() const noexcept;

        /**
         * Maximum depth
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float maxDepth() const noexcept;

    private:
        /**
         * Offset x / y
         */
        math::uivec2 _offset;

    public:
        /**
         * Offset x coordinate
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 offsetX() const noexcept;

        /**
         * Offset y coordinate
         *
         * @author Julius
         * @date 01.12.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 offsetY() const noexcept;
    };
}
