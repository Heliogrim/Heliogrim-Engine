#pragma once

#include "Geometry.hpp"
#include "LandscapeGeometryLayout.hpp"

namespace ember::engine::assets {

    class LandscapeGeometry :
        public Geometry<LandscapeGeometryLayout> {
    public:
        inline static const asset_type_id type_id { "LandscapeGeometry"_typeId };
    };
}
