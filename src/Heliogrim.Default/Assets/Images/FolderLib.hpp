#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderLib :
        public ImageAsset,
        public AssetAutoGuid<FolderLib> {
    public:
        FolderLib():
            ImageAsset(
                FolderLib::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-lib.ktx)"
            ) {}
    };
}
