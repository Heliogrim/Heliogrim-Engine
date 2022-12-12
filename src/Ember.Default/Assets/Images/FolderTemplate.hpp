#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderTemplate :
        public ImageAsset,
        public AssetAutoGuid<FolderTemplate> {
    public:
        FolderTemplate():
            ImageAsset(FolderTemplate::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-template.ktx)") {}
    };
}
