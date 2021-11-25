#pragma once
#include "SkeletalMeshComponent.hpp"

namespace ember {

    class SkinnedMeshComponent :
        public SkeletalMeshComponent {
    public:
        using this_type = SkinnedMeshComponent;
        using underlying_type = SkeletalMeshComponent;

    public:
    };
}
