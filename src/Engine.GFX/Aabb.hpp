#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    struct Aabb2d {
        math::fvec2 min;
        math::fvec2 max;

    public:
        [[nodiscard]] constexpr bool intersects(
            cref<Aabb2d> other_
        ) const noexcept {
            return (
                (min.x <= other_.max.x && max.x >= other_.min.x) &&
                (min.y <= other_.max.y && max.y >= other_.min.y)
            );
        }

        [[nodiscard]] constexpr bool intersects(
            cref<math::fvec2> point_
        ) const noexcept {
            return (
                (min.x <= point_.x && point_.x <= max.x) &&
                (min.y <= point_.y && point_.y <= max.y)
            );
        }

        [[nodiscard]] constexpr bool sphericalIntersects(
            math::fvec2 point_
        ) const noexcept {

            const auto halfExtent = (max / 2.F) - (min / 2.F);
            const auto delta = (min + halfExtent) - point_;
            // dot ( delta, delta ) < dot ( halfExtent, halfExtent )
            return (
                (delta.x * delta.x < halfExtent.x * halfExtent.x) ||
                (delta.y * delta.y < halfExtent.y * halfExtent.y)
            );
        }

        [[nodiscard]] constexpr bool sphericalIntersects(
            cref<math::fvec2> point_,
            math::fvec2::value_type radius_
        ) const noexcept {

            const auto halfExtent = (max / 2.F) - (min / 2.F);
            const auto delta = (min + halfExtent) - point_;
            const auto sqr = radius_ * radius_;

            // dot ( delta, delta ) + ( radius * radius ) < dot ( halfExtent, halfExtent )
            return (
                ((delta.x * delta.x - sqr) < halfExtent.x * halfExtent.x) ||
                ((delta.y * delta.y - sqr) < halfExtent.y * halfExtent.y)
            );
        }

    public:
        [[nodiscard]] constexpr Aabb2d join(cref<Aabb2d> other_) const noexcept {
            return {
                math::compMin<math::fvec2::value_type>(min, other_.min),
                math::compMax<math::fvec2::value_type>(max, other_.max)
            };
        }

        [[nodiscard]] constexpr Aabb2d overlap(cref<Aabb2d> other_) const noexcept {
            // Warning: This will return min > max for not intersecting aabbs
            return {
                math::compMax<math::fvec2::value_type>(min, other_.min),
                math::compMin<math::fvec2::value_type>(max, other_.max)
            };
        }
    };

    struct Aabb {
        math::fvec3 min;
        math::fvec3 max;

    public:
        [[nodiscard]] constexpr bool intersects(
            cref<Aabb> other_
        ) const noexcept {
            return (
                (min.x <= other_.max.x && max.x >= other_.min.x) &&
                (min.y <= other_.max.y && max.y >= other_.min.y) &&
                (min.z <= other_.max.z && max.z >= other_.min.z)
            );
        }

        [[nodiscard]] constexpr bool intersects(
            cref<math::fvec3> point_
        ) const noexcept {
            return (
                (min.x <= point_.x && point_.x <= max.x) &&
                (min.y <= point_.y && point_.y <= max.y) &&
                (min.z <= point_.z && point_.z <= max.z)
            );
        }

        [[nodiscard]] constexpr bool sphericalIntersects(
            cref<math::fvec3> point_
        ) const noexcept {

            const auto halfExtent = (max / 2.F) - (min / 2.F);
            const auto delta = (min + halfExtent) - point_;
            // dot ( delta, delta ) < dot ( halfExtent, halfExtent )
            return (
                (delta.x * delta.x < halfExtent.x * halfExtent.x) ||
                (delta.y * delta.y < halfExtent.y * halfExtent.y) ||
                (delta.z * delta.z < halfExtent.z * halfExtent.z)
            );
        }

        [[nodiscard]] constexpr bool sphericalIntersects(
            cref<math::fvec3> point_,
            math::fvec3::value_type radius_
        ) const noexcept {

            const auto halfExtent = (max / 2.F) - (min / 2.F);
            const auto delta = (min + halfExtent) - point_;
            const auto sqr = radius_ * radius_;

            // dot ( delta, delta ) + ( radius * radius ) < dot ( halfExtent, halfExtent )
            return (
                ((delta.x * delta.x - sqr) < halfExtent.x * halfExtent.x) ||
                ((delta.y * delta.y - sqr) < halfExtent.y * halfExtent.y) ||
                ((delta.z * delta.z - sqr) < halfExtent.z * halfExtent.z)
            );
        }

    public:
        [[nodiscard]] constexpr Aabb join(cref<Aabb> other_) const noexcept {
            return {
                math::compMin<math::fvec3::value_type>(min, other_.min),
                math::compMax<math::fvec3::value_type>(max, other_.max)
            };
        }

        [[nodiscard]] constexpr Aabb overlap(cref<Aabb> other_) const noexcept {
            // Warning: This will return min > max for not intersecting aabbs
            return {
                math::compMax<math::fvec3::value_type>(min, other_.min),
                math::compMin<math::fvec3::value_type>(max, other_.max)
            };
        }
    };

    using Aabb3d = Aabb;
}
