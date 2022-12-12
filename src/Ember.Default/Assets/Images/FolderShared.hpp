#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderShared :
        public ImageAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared():
            ImageAsset(FolderShared::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-shared.ktx)") {}
    };
}
