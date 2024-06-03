#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class FolderAudio :
        public ImageAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio():
            ImageAsset(
                clone(FolderAudio::unstable_auto_guid()),
                R"(resources\imports\ktx\folder-audio.ktx)"
            ) {}
    };
}
