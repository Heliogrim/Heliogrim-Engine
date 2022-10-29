#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderScripts :
        public ImageAsset,
        public AssetAutoGuid<FolderScripts> {
    public:
        FolderScripts():
            ImageAsset(FolderScripts::auto_guid(),
                R"(resources\imports\ktx\folder-scripts.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderScripts)
}
