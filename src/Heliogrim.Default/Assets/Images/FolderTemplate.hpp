#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderTemplate :
        public ImageAsset,
        public AssetAutoGuid<FolderTemplate> {
    public:
        FolderTemplate():
            ImageAsset(
                FolderTemplate::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-template.ktx)"
            ) {}
    };
}
