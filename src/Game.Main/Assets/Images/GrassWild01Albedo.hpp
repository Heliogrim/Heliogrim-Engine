#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
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
