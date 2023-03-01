#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderPackages :
        public ImageAsset,
        public AssetAutoGuid<FolderPackages> {
    public:
        FolderPackages():
            ImageAsset(
                FolderPackages::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-packages.ktx)"
            ) {}
    };
}
