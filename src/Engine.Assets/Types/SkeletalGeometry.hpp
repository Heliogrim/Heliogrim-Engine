#pragma once

#include "Geometry.hpp"
#include "SkeletalGeometryLayout.hpp"

namespace ember::engine::assets {

    class SkeletalGeometry :
        public Geometry<SkeletalGeometryLayout> {
    public:
        inline static const asset_type_id type_id { "SkeletalGeometry"_typeId };
    };
}
