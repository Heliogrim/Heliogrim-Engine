#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderCore :
        public ImageAsset,
        public AssetAutoGuid<FolderCore> {
    public:
        FolderCore():
            ImageAsset(
                FolderCore::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-core.ktx)"
            ) {}
    };
}
