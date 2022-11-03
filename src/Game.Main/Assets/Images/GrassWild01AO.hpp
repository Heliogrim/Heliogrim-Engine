#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01AO :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01AO> {
    public:
        GrassWild01AO() :
            ImageAsset(GrassWild01AO::unstable_auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01AO)

}
