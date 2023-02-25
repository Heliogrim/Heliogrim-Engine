#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class DefaultAlpha :
        public ImageAsset,
        public AssetAutoGuid<DefaultAlpha> {
    public:
        DefaultAlpha():
            ImageAsset(
                DefaultAlpha::unstable_auto_guid(),
                R"(resources\imports\ktx\default_alpha.ktx)"
            ) {}
    };
}
