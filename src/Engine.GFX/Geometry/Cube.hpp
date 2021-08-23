#pragma once
#include <Engine.Common/Math/Vector.hpp>

#include "Geometry.hpp"

namespace ember::engine::gfx {
    class Cube :
        public Geometry {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        Cube() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        ~Cube() override;

        /**
         * Convert parametric Cube to Mesh
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A pointer to a Mesh.
         */
        [[nodiscard]] ptr<Mesh> convert() const override;

    private:
        /**
         * Segments
         */
        math::uivec2 _segment;

    public:
        /**
         * Gets the segments
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;math::uivec2&gt;
         */
        [[nodiscard]] ref<math::uivec2> segments() noexcept;

    private:
        /**
         * Scale
         */
        math::fvec2 _scale;

    public:
        /**
         * Gets the scale
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;math::fvec2&gt;
         */
        [[nodiscard]] ref<math::fvec2> scale() noexcept;
    };
}
