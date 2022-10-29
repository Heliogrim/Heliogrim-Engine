#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderFont :
        public ImageAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont():
            ImageAsset(FolderFont::auto_guid(),
                R"(resources\imports\ktx\folder-font.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderFont)
}
