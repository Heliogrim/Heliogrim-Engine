#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class WoodenBucket02 :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenBucket02> {
    public:
        WoodenBucket02();
    };
}
