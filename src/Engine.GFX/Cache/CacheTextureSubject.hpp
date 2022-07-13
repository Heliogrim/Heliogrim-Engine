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

            if (extent != other_.extent) {
                // TODO: Replace unsafe float pointer equality check with epsilon check
                return extent.sqrLength() < other_.extent.sqrLength();
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

            if (extent != other_.extent) {
                // TODO: Replace unsafe float pointer equality check with epsilon check
                return extent.sqrLength() > other_.extent.sqrLength();
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

            if (extent != other_.extent) {
                // TODO: Replace unsafe float pointer equality check with epsilon check
                return extent.sqrLength() < other_.extent.sqrLength();
            }

            return extent == other_.extent;
        }

        [[nodiscard]] bool operator>=(cref<CacheTextureSubject> other_) const noexcept {
            if (layer < other_.layer) {
                return false;
            }

            if (mip < other_.mip) {
                return false;
            }

            if (extent != other_.extent) {
                // TODO: Replace unsafe float pointer equality check with epsilon check
                return extent.sqrLength() > other_.extent.sqrLength();
            }

            return extent == other_.extent;
        }
    };

    static_assert(_STD totally_ordered<CacheTextureSubject>);

    using TextureSubResource = CacheTextureSubject;
}
