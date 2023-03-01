#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DefaultAO :
        public ImageAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO():
            ImageAsset(DefaultAO::unstable_auto_guid(),
                R"(resources\imports\ktx\default_ao.ktx)") {}
    };
}
