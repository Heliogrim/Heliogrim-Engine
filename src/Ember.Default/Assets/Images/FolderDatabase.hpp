#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderDatabase :
        public ImageAsset,
        public AssetAutoGuid<FolderDatabase> {
    public:
        FolderDatabase():
            ImageAsset(FolderDatabase::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-database.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderDatabase)
}
