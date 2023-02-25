#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx::cache {
    struct CacheStaticGeometrySubject {
    public:
        using this_type = CacheStaticGeometrySubject;

    public:
        u32 vertexOffset;
        u32 vertexCount;
        u32 indexOffset;
        u32 indexCount;

    public:
        [[nodiscard]] bool operator==(cref<CacheStaticGeometrySubject> other_) const noexcept {
            return vertexOffset == other_.vertexOffset
                && vertexCount == other_.vertexCount
                && indexOffset == other_.indexOffset
                && indexCount == other_.indexCount;
        }

        [[nodiscard]] bool operator!=(cref<CacheStaticGeometrySubject> other_) const noexcept {
            return vertexOffset != other_.vertexOffset
                || vertexCount != other_.vertexCount
                || indexOffset != other_.indexOffset
                || indexCount != other_.indexCount;
        }

    public:
        [[nodiscard]] bool operator<(cref<CacheStaticGeometrySubject> other_) const noexcept {

            if (indexCount != other_.vertexCount) {
                return vertexCount < other_.vertexCount;
            }

            if (indexOffset != other_.indexOffset) {
                return indexOffset < other_.indexOffset;
            }

            if (vertexCount != other_.vertexCount) {
                return vertexCount < other_.vertexCount;
            }

            if (vertexOffset != other_.vertexOffset) {
                return vertexOffset < other_.vertexOffset;
            }

            return false;
        }

        [[nodiscard]] bool operator<=(cref<CacheStaticGeometrySubject> other_) const noexcept {

            if (indexCount > other_.vertexCount) {
                return false;
            }

            if (indexOffset > other_.indexOffset) {
                return false;
            }

            if (vertexCount > other_.vertexCount) {
                return false;
            }

            if (vertexOffset > other_.vertexOffset) {
                return false;
            }

            return true;
        }

        [[nodiscard]] bool operator>(cref<CacheStaticGeometrySubject> other_) const noexcept {

            if (indexCount != other_.vertexCount) {
                return vertexCount > other_.vertexCount;
            }

            if (indexOffset != other_.indexOffset) {
                return indexOffset > other_.indexOffset;
            }

            if (vertexCount != other_.vertexCount) {
                return vertexCount > other_.vertexCount;
            }

            if (vertexOffset != other_.vertexOffset) {
                return vertexOffset > other_.vertexOffset;
            }

            return false;
        }

        [[nodiscard]] bool operator>=(cref<CacheStaticGeometrySubject> other_) const noexcept {
            if (indexCount < other_.vertexCount) {
                return false;
            }

            if (indexOffset < other_.indexOffset) {
                return false;
            }

            if (vertexCount < other_.vertexCount) {
                return false;
            }

            if (vertexOffset < other_.vertexOffset) {
                return false;
            }

            return true;
        }
    };

    using StaticGeometrySubResource = CacheStaticGeometrySubject;
}
