#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class WoodenPier01Poles :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenPier01Poles> {
    public:
        WoodenPier01Poles();
    };
}
