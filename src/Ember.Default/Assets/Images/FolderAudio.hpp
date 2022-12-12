#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class FolderAudio :
        public ImageAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio():
            ImageAsset(FolderAudio::unstable_auto_guid(),
                R"(resources\imports\ktx\folder-audio.ktx)") {}
    };
}
