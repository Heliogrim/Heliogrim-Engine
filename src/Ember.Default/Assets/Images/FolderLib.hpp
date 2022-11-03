#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderLib :
        public ImageAsset,
        public AssetAutoGuid<FolderLib> {
    public:
        FolderLib():
            ImageAsset(FolderLib::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-lib.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderLib)
}
