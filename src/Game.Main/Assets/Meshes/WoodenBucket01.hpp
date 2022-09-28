#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class WoodenBucket01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenBucket01> {
    public:
        WoodenBucket01();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01);
}
