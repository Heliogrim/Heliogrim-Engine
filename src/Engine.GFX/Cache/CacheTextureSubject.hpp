#pragma once

#include <concepts>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace ember::engine::gfx::cache {

    struct CacheTextureSubject {
    public:
        using this_type = CacheTextureSubject;

    public:
        u32 layer;
        u32 mip;
        math::uivec3 offset;
        math::uivec3 extent;

    public:
        [[nodiscard]] bool operator==(cref<CacheTextureSubject> other_) const noexcept {
            return layer == other_.layer && mip == other_.mip && offset == other_.offset && extent == other_.extent;
        }

        [[nodiscard]] bool operator!=(cref<CacheTextureSubject> other_) const noexcept {
            return layer != other_.layer || mip != other_.mip || offset != other_.offset || extent != other_.extent;
        }

    public:
        [[nodiscard]] bool operator<(cref<CacheTextureSubject> other_) const noexcept {
            if (layer != other_.layer) {
                return layer < other_.layer;
            }

            if (mip != other_.mip) {
                return mip < other_.mip;
            }

            if (offset != other_.offset) {
                if (offset.x < other_.offset.x)
                    return true;
                if (offset.y < other_.offset.y)
                    return true;
                if (offset.z < other_.offset.z)
                    return true;
            }

            if (extent != other_.extent) {
                if (extent.x < other_.extent.x)
                    return true;
                if (extent.y < other_.extent.y)
                    return true;
                if (extent.z < other_.extent.z)
                    return true;
            }

            return false;
        }

        [[nodiscard]] bool operator>(cref<CacheTextureSubject> other_) const noexcept {
            if (layer != other_.layer) {
                return layer > other_.layer;
            }

            if (mip != other_.mip) {
                return mip > other_.mip;
            }

            if (offset != other_.offset) {
                if (offset.x > other_.offset.x)
                    return true;
                if (offset.y > other_.offset.y)
                    return true;
                if (offset.z > other_.offset.z)
                    return true;
            }

            if (extent != other_.extent) {
                if (extent.x > other_.extent.x)
                    return true;
                if (extent.y > other_.extent.y)
                    return true;
                if (extent.z > other_.extent.z)
                    return true;
            }

            return false;
        }

        [[nodiscard]] bool operator<=(cref<CacheTextureSubject> other_) const noexcept {
            if (layer > other_.layer) {
                return false;
            }

            if (mip > other_.mip) {
                return false;
            }

            if (offset != other_.offset) {
                if (offset.x > other_.offset.x)
                    return false;
                if (offset.y > other_.offset.y)
                    return false;
                if (offset.z > other_.offset.z)
                    return false;
            }

            if (extent != other_.extent) {
                if (extent.x > other_.extent.x)
                    return false;
                if (extent.y > other_.extent.y)
                    return false;
                if (extent.z > other_.extent.z)
                    return false;
            }

            return offset == other_.offset && extent == other_.extent;
        }

        [[nodiscard]] bool operator>=(cref<CacheTextureSubject> other_) const noexcept {
            if (layer < other_.layer) {
                return false;
            }

            if (mip < other_.mip) {
                return false;
            }

            if (offset != other_.offset) {
                if (offset.x < other_.offset.x)
                    return false;
                if (offset.y < other_.offset.y)
                    return false;
                if (offset.z < other_.offset.z)
                    return false;
            }

            if (extent != other_.extent) {
                if (extent.x < other_.extent.x)
                    return false;
                if (extent.y < other_.extent.y)
                    return false;
                if (extent.z < other_.extent.z)
                    return false;
            }

            return offset == other_.offset && extent == other_.extent;
        }
    };

    static_assert(_STD totally_ordered<CacheTextureSubject>);

    using TextureSubResource = CacheTextureSubject;
}
