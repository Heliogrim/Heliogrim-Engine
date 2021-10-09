#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Roughness :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Roughness() :
            ImageAsset(GrassWild01Roughness::guid) {}
    };

}
