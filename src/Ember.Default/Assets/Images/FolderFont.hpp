#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderFont :
        public ImageAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont():
            ImageAsset(
                FolderFont::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-font.ktx)"
            ) {}
    };
}
