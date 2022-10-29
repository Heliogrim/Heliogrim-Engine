#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderGenerator :
        public ImageAsset,
        public AssetAutoGuid<FolderGenerator> {
    public:
        FolderGenerator():
            ImageAsset(FolderGenerator::auto_guid(),
                R"(resources\imports\ktx\folder-generator.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderGenerator)
}
