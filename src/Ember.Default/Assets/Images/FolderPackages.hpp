#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderPackages :
        public ImageAsset,
        public AssetAutoGuid<FolderPackages> {
    public:
        FolderPackages():
            ImageAsset(FolderPackages::auto_guid(),
                R"(resources\imports\ktx\folder-packages.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderPackages)
}
