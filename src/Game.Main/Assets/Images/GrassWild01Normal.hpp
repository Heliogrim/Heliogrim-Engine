#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Normal :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Normal() :
            ImageAsset(GrassWild01Normal::guid) {}
    };

}
