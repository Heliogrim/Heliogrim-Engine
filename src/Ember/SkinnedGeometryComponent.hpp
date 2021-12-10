#pragma once
#include "SkeletalGeometryComponent.hpp"

namespace ember {

    class SkinnedGeometryComponent :
        public SkeletalGeometryComponent {
    public:
        using this_type = SkinnedGeometryComponent;
        using underlying_type = SkeletalGeometryComponent;

        inline constexpr static component_type_id type_id { "SkinnedGeometryComponent"_typeId };

    public:
    };
}
