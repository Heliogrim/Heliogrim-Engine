#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderConfig :
        public ImageAsset,
        public AssetAutoGuid<FolderConfig> {
    public:
        FolderConfig():
            ImageAsset(FolderConfig::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-config.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderConfig)
}
