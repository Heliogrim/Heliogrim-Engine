#pragma once

#include "../Types.hpp"

namespace ember::math {
    /**
     * An extent 2d.
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class Extent2D {
    public:
        /** Default constructor */
        Extent2D() :
            width(static_cast<T>(0)),
            height(static_cast<T>(0)),
            offsetX(static_cast<T>(0)),
            offsetY(static_cast<T>(0)) { }

        /**
         * Constructor
         *
         * @param width_  The width.
         * @param height_ The height.
         */
        Extent2D(const T& width_, const T& height_) :
            width(width_),
            height(height_),
            offsetX(static_cast<T>(0)),
            offsetY(static_cast<T>(0)) { }

        /**
         * Constructor
         *
         * @param width_   The width.
         * @param height_  The height.
         * @param offsetX_ The offset x coordinate.
         * @param offsetY_ The offset y coordinate.
         */
        Extent2D(const T& width_, const T& height_, const T& offsetX_, const T& offsetY_) :
            width(width_),
            height(height_),
            offsetX(offsetX_),
            offsetY(offsetY_) { }

        T width;
        T height;
        T offsetX;
        T offsetY;

        /**
         * Sets an extent
         *
         * @param height_ The height.
         * @param width_  The width.
         */
        void setExtent(const T& height_, const T& width_) {
            height = height_;
            width = width_;
        }

        /**
         * Sets a height
         *
         * @param height_ The height.
         */
        void setHeight(const T& height_) {
            height = height_;
        }

        /**
         * Sets a width
         *
         * @param width_ The width.
         */
        void setWidth(const T& width_) {
            width = width_;
        }

        /**
         * Sets an offset
         *
         * @param offsetX_ The offset x coordinate.
         * @param offsetY_ The offset y coordinate.
         */
        void setOffset(const T& offsetX_, const T& offsetY_) {
            offsetX = offsetX_;
            offsetY = offsetY_;
        }

        /**
         * Sets offset x coordinate
         *
         * @param offset_ The offset.
         */
        void setOffsetX(const T& offset_) {
            offsetX = offset_;
        }

        /**
         * Sets offset y coordinate
         *
         * @param offset_ The offset.
         */
        void setOffsetY(const T& offset_) {
            offsetY = offset_;
        }

        /**
         * Equality operator
         *
         * @param other_ The extent to compare with
         *
         * @returns True is all values are evaluated equal.
         */
        [[nodiscard]] bool operator==(const Extent2D<T>& other_) const noexcept {
            return width == other_.width && height == other_.height &&
                offsetX == other_.offsetX && offsetY == other_.offsetY;
        }
    };

    /**
     * An extent 3d.
     *
     * @tparam T Generic type parameter.
     */
    template <typename T>
    class Extent3D {
    public:
        using value_type = T;
        using off_type = T;

    public:
        /** Default constructor */
        Extent3D() :
            width(static_cast<T>(0)),
            height(static_cast<T>(0)),
            depth(static_cast<T>(0)),
            offsetX(static_cast<T>(0)),
            offsetY(static_cast<T>(0)),
            offsetZ(static_cast<T>(0)) { }

        /**
         * Constructor
         *
         * @param width_  The width.
         * @param height_ The height.
         * @param depth_  (Optional) The depth.
         */
        Extent3D(const T& width_, const T& height_, const T& depth_ = static_cast<T>(1)) :
            width(width_),
            height(height_),
            depth(depth_),
            offsetX(static_cast<T>(0)),
            offsetY(static_cast<T>(0)),
            offsetZ(static_cast<T>(0)) { }

        /**
         * Constructor
         *
         * @param width_   The width.
         * @param height_  The height.
         * @param depth_   The depth.
         * @param offsetX_ The offset x coordinate.
         * @param offsetY_ The offset y coordinate.
         * @param offsetZ_ The offset z coordinate.
         */
        Extent3D(const T& width_, const T& height_, const T& depth_, const T& offsetX_, const T& offsetY_,
            const T& offsetZ_) :
            width(width_),
            height(height_),
            depth(depth_),
            offsetX(offsetX_),
            offsetY(offsetY_),
            offsetZ(offsetZ_) { }

        value_type width;
        value_type height;
        value_type depth;
        off_type offsetX;
        off_type offsetY;
        off_type offsetZ;

        /**
         * Sets an extent
         *
         * @param height_ The height.
         * @param width_  The width.
         * @param depth_  (Optional) The depth.
         */
        void setExtent(const T& height_, const T& width_, const T& depth_ = static_cast<T>(1)) {
            height = height_;
            width = width_;
            depth = depth_;
        }

        /**
         * Sets a depth
         *
         * @param depth_ The depth.
         */
        void setDepth(const T& depth_) {
            depth = depth_;
        }

        /**
         * Sets a height
         *
         * @param height_ The height.
         */
        void setHeight(const T& height_) {
            height = height_;
        }

        /**
         * Sets a width
         *
         * @param width_ The width.
         */
        void setWidth(const T& width_) {
            width = width_;
        }

        /**
         * Sets an offset
         *
         * @param offsetX_ The offset x coordinate.
         * @param offsetY_ The offset y coordinate.
         * @param offsetZ_ (Optional) The offset z coordinate.
         */
        void setOffset(const T& offsetX_, const T& offsetY_, const T& offsetZ_ = static_cast<T>(0)) {
            offsetX = offsetX_;
            offsetY = offsetY_;
            offsetZ = offsetZ_;
        }

        /**
         * Sets offset x coordinate
         *
         * @param offset_ The offset.
         */
        void setOffsetX(const T& offset_) {
            offsetX = offset_;
        }

        /**
         * Sets offset y coordinate
         *
         * @param offset_ The offset.
         */
        void setOffsetY(const T& offset_) {
            offsetY = offset_;
        }

        /**
         * Sets offset z coordinate
         *
         * @param offset_ The offset.
         */
        void setOffsetZ(const T& offset_) {
            offsetZ = offset_;
        }
    };

    typedef Extent2D<s32> iExtent2D;
    typedef Extent2D<u32> uExtent2D;
    typedef Extent2D<float> fExtent2D;
    typedef Extent3D<s32> iExtent3D;
    typedef Extent3D<u32> uExtent3D;
    typedef Extent3D<float> fExtent3D;
}
