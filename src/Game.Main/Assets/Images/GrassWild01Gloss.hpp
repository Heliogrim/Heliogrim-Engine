#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Gloss :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Gloss() :
            ImageAsset(GrassWild01Gloss::guid) {}
    };

}
