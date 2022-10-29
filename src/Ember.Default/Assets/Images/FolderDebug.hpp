#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderDebug :
        public ImageAsset,
        public AssetAutoGuid<FolderDebug> {
    public:
        FolderDebug():
            ImageAsset(FolderDebug::auto_guid(),
                R"(resources\imports\ktx\folder-debug.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderDebug)
}
