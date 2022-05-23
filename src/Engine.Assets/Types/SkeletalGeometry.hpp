#pragma once

#include "Geometry.hpp"
#include "SkeletalGeometryLayout.hpp"

namespace ember::engine::assets {

    class SkeletalGeometry :
        public Geometry<SkeletalGeometryLayout> {
    public:
        inline static const asset_type_id typeId { "SkeletalGeometry"_typeId };
    };
}
