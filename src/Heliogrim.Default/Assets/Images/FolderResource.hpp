#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderResource :
        public ImageAsset,
        public AssetAutoGuid<FolderResource> {
    public:
        FolderResource():
            ImageAsset(
                FolderResource::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-resource.ktx)"
            ) {}
    };
}
