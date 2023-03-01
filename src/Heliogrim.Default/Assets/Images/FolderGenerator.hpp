#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderGenerator :
        public ImageAsset,
        public AssetAutoGuid<FolderGenerator> {
    public:
        FolderGenerator():
            ImageAsset(
                FolderGenerator::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-generator.ktx)"
            ) {}
    };
}
