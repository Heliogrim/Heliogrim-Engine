#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class WoodenBucket02 :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenBucket02> {
    public:
        WoodenBucket02();
    };
}
