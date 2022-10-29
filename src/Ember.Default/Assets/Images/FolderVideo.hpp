#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderVideo :
        public ImageAsset,
        public AssetAutoGuid<FolderVideo> {
    public:
        FolderVideo():
            ImageAsset(FolderVideo::auto_guid(),
                R"(resources\imports\ktx\folder-video.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderVideo)
}
