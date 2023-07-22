#pragma once

#include "Geometry.hpp"

namespace hg::engine::assets {
    class SkeletalGeometry :
        public InheritMeta<SkeletalGeometry, Geometry> {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "SkeletalGeometry"_typeId };
    };
}
