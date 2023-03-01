#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderAnimation :
        public ImageAsset,
        public AssetAutoGuid<FolderAnimation> {
    public:
        FolderAnimation():
            ImageAsset(
                FolderAnimation::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-animation.ktx)"
            ) {}
    };
}
