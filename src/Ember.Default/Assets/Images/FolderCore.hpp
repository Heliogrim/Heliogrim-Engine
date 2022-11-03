#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderCore :
        public ImageAsset,
        public AssetAutoGuid<FolderCore> {
    public:
        FolderCore():
            ImageAsset(FolderCore::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-core.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderCore)
}
