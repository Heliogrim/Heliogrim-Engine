#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
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
