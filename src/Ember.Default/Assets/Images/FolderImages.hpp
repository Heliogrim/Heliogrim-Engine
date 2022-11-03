#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderImages :
        public ImageAsset,
        public AssetAutoGuid<FolderImages> {
    public:
        FolderImages():
            ImageAsset(FolderImages::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-images.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderImages)
}
