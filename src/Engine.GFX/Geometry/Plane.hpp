#pragma once

#include "Geometry.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx {
    class Plane :
        public Geometry {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        Plane();

        /**
         * Destructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        ~Plane() override;

        /**
         * Convert parametric Plane to Mesh
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A pointer to a Mesh.
         */
        [[nodiscard]] ptr<Mesh> convert() const override;

    private:
        /**
         * Gridding
         */
        math::uivec2 _grid;

    public:
        /**
         * Gets the grid
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A ref&lt;math::uivec2&gt;
         */
        [[nodiscard]] ref<math::uivec2> grid() noexcept;

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
