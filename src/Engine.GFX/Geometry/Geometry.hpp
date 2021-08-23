#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Mesh.hpp"

namespace ember::engine::gfx {
    class Geometry {
    public:
        using value_type = Geometry;
        using reference_type = Geometry&;
        using const_reference_type = const Geometry&;

        /**
         * Destructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        virtual ~Geometry() = default;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 19.02.2021
         */
        Geometry() = default;

    public:
        /**
         * Gets the convert
         *
         * @author Julius
         * @date 19.02.2021
         *
         * @returns A Mesh.
         */
        [[nodiscard]] virtual ptr<Mesh> convert() const = 0;

    };
}
