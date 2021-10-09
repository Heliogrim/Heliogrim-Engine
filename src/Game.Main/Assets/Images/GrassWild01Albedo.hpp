#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Albedo :
        public ImageAsset {
    public:
        inline static const asset_guid guid = generate_asset_guid();

    public:
        GrassWild01Albedo() :
            ImageAsset(GrassWild01Albedo::guid) {}
    };

}
