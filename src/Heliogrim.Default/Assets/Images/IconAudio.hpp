#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconAudio :
        public ImageAsset,
        public AssetAutoGuid<IconAudio> {
    public:
        IconAudio():
            ImageAsset(
                IconAudio::unstable_auto_guid(),
                R"(resources\imports\ktx\audio.ktx)"
            ) {}
    };
}
