#pragma once

#include "Geometry.hpp"

namespace ember::engine::gfx {
    class Sphere :
        public Geometry {
    public:

        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.02.2021
         */
        Sphere() = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.02.2021
         */
        ~Sphere() override;

        /**
         * Convert parametric Sphere to Mesh
         *
         * @author Julius
         * @date 20.02.2021
         *
         * @returns A pointer to a Mesh.
         */
        [[nodiscard]] ptr<Mesh> convert() const override;

    };
}
