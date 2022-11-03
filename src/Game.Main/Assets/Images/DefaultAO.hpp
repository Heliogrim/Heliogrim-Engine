#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultAO :
        public ImageAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO():
            ImageAsset(DefaultAO::unstable_auto_guid(),
                R"(resources\imports\ktx\default_ao.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultAO)
}
