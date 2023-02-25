#pragma once

#include "Geometry.hpp"

namespace ember::engine::assets {
    class SkeletalGeometry :
        public Geometry {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "SkeletalGeometry"_typeId };
    };
}
