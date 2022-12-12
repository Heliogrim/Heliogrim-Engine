#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderError :
        public ImageAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError():
            ImageAsset(FolderError::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-error.ktx)") {}
    };
}
