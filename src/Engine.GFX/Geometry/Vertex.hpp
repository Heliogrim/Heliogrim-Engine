#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {

    /**
     * A vertex with template attribute types
     *
     * @author Julius
     * @date 19.11.2020
     *
     * @tparam pt Type of the position vector.
     * @tparam ct Type of the color {vector}
     * @tparam uvmt Type of the uvm mapping {vector}.
     * @tparam nt Type of the normal vector.
     */
    template <
        typename pt = float,
        typename ct = u8,
        typename uvmt = float,
        typename nt = float>
    struct vertex_t {

        using position_type = math::vec3_t<pt>;
        using color_type = math::vec3_t<ct>;
        using uvm_type = math::vec3_t<uvmt>;
        using normal_type = math::vec3_t<nt>;

        vertex_t() = default;

        vertex_t(cref<position_type> position_, cref<color_type> color_, cref<uvm_type> uvm_,
            cref<normal_type> normal_) :
            position(position_),
            color(color_),
            uvm(uvm_),
            normal(normal_) { }

        /** The position */
        position_type position;

        /** The color */
        color_type color;

        /** The uvm */
        uvm_type uvm;

        /** The normal */
        normal_type normal;
    };

    template <typename pt, typename ct, typename uvmt, typename nt>
    bool operator==(const vertex_t<pt, ct, uvmt, nt>& left_, const vertex_t<pt, ct, uvmt, nt>& right_) {
        return left_.pos == right_.pos &&
            left_.color == right_.color &&
            left_.texCoord == right_.texCoord &&
            left_.normal == right_.normal;
    }

    /** Defines an alias representing the vertex */
    typedef vertex_t<float, u8, float, float> vertex;
}
