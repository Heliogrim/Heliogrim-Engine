#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Displacement :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Displacement() :
            ImageAsset(GrassWild01Displacement::guid) {}
    };

}
