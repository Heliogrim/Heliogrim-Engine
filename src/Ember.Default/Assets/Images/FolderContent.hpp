#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderContent :
        public ImageAsset,
        public AssetAutoGuid<FolderContent> {
    public:
        FolderContent():
            ImageAsset(FolderContent::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-content.ktx)") {}
    };
}
