#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class WoodenBucket01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenBucket01> {
    public:
        WoodenBucket01();
    };
}
