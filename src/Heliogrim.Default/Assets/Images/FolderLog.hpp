#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
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
