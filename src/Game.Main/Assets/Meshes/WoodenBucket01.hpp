#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class WoodenBucket01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenBucket01> {
    public:
        WoodenBucket01();
    };
}
