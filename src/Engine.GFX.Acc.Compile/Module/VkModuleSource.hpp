#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>

#include "ModuleSource.hpp"
#include "VkBindingLocation.hpp"

namespace hg::engine::gfx::acc {
    struct VkModuleSource :
        public ModuleSource {
    public:
        using this_type = VkModuleSource;
        DenseMap<TransferToken, VkBindingLocation> mappedLocations;
    };
}
