#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderError :
        public ImageAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError():
            ImageAsset(
                FolderError::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-error.ktx)"
            ) {}
    };
}
