#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class DefaultRoughness :
        public ImageAsset,
        public AssetAutoGuid<DefaultRoughness> {
    public:
        DefaultRoughness():
            ImageAsset(
                DefaultRoughness::unstable_auto_guid(),
                R"(resources\imports\ktx\default_roughness.ktx)"
            ) {}
    };
}
