#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderImport :
        public ImageAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport():
            ImageAsset(FolderImport::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-import.ktx)") {}
    };
}
