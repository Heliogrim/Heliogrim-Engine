#pragma once
#include "ModelComponent.hpp"

namespace ember {

    class SkeletalGeometryComponent :
        public ModelComponent {
    public:
        using this_type = SkeletalGeometryComponent;
        using underlying_type = ModelComponent;

        inline constexpr static component_type_id typeId { "SkeletalGeometryComponent"_typeId };

    public:
    };
}
