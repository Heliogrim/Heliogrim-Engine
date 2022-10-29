#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderAudio :
        public ImageAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio():
            ImageAsset(FolderAudio::auto_guid(),
                R"(resources\imports\ktx\folder-audio.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderAudio)
}
