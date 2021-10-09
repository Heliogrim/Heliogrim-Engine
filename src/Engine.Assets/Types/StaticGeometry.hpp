#pragma once

#include "Geometry.hpp"
#include "StaticGeometryLayout.hpp"

namespace ember::engine::assets {

    class StaticGeometry :
        public Geometry<StaticGeometryLayout> {
    public:
        inline static const asset_type_id type_id { "StaticGeometry"_typeId };
    };
}
