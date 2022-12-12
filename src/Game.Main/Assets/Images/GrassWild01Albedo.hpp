#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class GrassWild01Albedo :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Albedo> {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.10.2021
         */
        GrassWild01Albedo();
    };
}
