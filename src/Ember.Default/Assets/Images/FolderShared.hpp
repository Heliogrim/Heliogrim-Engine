#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderShared :
        public ImageAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared():
            ImageAsset(FolderShared::auto_guid(),
                R"(resources\imports\ktx\folder-shared.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderShared)
}
