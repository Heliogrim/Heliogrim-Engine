#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderError :
        public ImageAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError():
            ImageAsset(FolderError::auto_guid(),
                R"(resources\imports\ktx\folder-error.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderError)
}
