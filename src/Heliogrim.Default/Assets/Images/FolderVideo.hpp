#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderVideo :
        public ImageAsset,
        public AssetAutoGuid<FolderVideo> {
    public:
        FolderVideo():
            ImageAsset(
                FolderVideo::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-video.ktx)"
            ) {}
    };
}
