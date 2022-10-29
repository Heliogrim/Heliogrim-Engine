#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderMeta :
        public ImageAsset,
        public AssetAutoGuid<FolderMeta> {
    public:
        FolderMeta():
            ImageAsset(FolderMeta::auto_guid(),
                R"(resources\imports\ktx\folder-meta.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderMeta)
}
