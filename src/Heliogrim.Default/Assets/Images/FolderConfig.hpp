#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderConfig :
        public ImageAsset,
        public AssetAutoGuid<FolderConfig> {
    public:
        FolderConfig():
            ImageAsset(
                FolderConfig::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-config.ktx)"
            ) {}
    };
}
