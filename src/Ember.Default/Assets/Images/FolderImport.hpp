#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderImport :
        public ImageAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport():
            ImageAsset(FolderImport::auto_guid(),
                R"(resources\imports\ktx\folder-import.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderImport)
}
