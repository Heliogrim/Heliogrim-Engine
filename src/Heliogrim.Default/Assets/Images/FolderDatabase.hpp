#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderDatabase :
        public ImageAsset,
        public AssetAutoGuid<FolderDatabase> {
    public:
        FolderDatabase():
            ImageAsset(
                FolderDatabase::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-database.ktx)"
            ) {}
    };
}
