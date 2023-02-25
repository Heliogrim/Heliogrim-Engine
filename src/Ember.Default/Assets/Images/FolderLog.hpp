#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderLog :
        public ImageAsset,
        public AssetAutoGuid<FolderLog> {
    public:
        FolderLog():
            ImageAsset(
                FolderLog::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-log.ktx)"
            ) {}
    };
}
