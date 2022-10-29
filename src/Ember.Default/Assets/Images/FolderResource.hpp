#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderResource :
        public ImageAsset,
        public AssetAutoGuid<FolderResource> {
    public:
        FolderResource():
            ImageAsset(FolderResource::auto_guid(),
                R"(resources\imports\ktx\folder-resource.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderResource)
}
