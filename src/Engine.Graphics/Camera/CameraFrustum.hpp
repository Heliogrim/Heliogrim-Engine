#pragma once

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Vector.hpp>

namespace hg::engine::gfx {
    class CameraFrustum {
    public:
        enum FrustumSide {
            eLeft = 0x0,
            eRight = 0x1,
            eTop = 0x2,
            eBottom = 0x3,
            eBack = 0x4,
            eFront = 0x5
        };

        /**
         * Updates the given src_
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  src_ Source for the.
         */
        void update(math::mat4 src_);

        /**
         * Check spherical
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @param  origin_ The origin.
         * @param  radius_ The radius.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool checkSpherical(const math::vec3& origin_, const float radius_) const noexcept;

    private:
        std::array<math::vec4, 6> _planes;
    };
}
