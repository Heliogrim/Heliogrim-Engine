#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderShared :
        public ImageAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared():
            ImageAsset(
                FolderShared::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-shared.ktx)"
            ) {}
    };
}
