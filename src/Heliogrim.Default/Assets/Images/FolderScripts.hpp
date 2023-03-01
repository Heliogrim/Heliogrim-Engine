#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderScripts :
        public ImageAsset,
        public AssetAutoGuid<FolderScripts> {
    public:
        FolderScripts():
            ImageAsset(
                FolderScripts::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-scripts.ktx)"
            ) {}
    };
}
