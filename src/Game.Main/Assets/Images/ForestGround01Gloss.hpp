#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Gloss :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        ForestGround01Gloss() :
            ImageAsset(ForestGround01Gloss::guid) {}
    };

}
