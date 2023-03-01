#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class WoodenPier01Planks :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenPier01Planks> {
    public:
        WoodenPier01Planks();
    };
}
