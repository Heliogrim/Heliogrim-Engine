#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderEnvironment :
        public ImageAsset,
        public AssetAutoGuid<FolderEnvironment> {
    public:
        FolderEnvironment():
            ImageAsset(FolderEnvironment::auto_guid(),
                R"(resources\imports\ktx\folder-environment.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderEnvironment)
}
