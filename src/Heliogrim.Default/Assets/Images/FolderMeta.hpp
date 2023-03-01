#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderMeta :
        public ImageAsset,
        public AssetAutoGuid<FolderMeta> {
    public:
        FolderMeta():
            ImageAsset(
                FolderMeta::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-meta.ktx)"
            ) {}
    };
}
