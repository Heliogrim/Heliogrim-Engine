#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class WoodenPier01Planks :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenPier01Planks> {
    public:
        WoodenPier01Planks();
    };

    AUTO_REGISTER_ASSET(WoodenPier01Planks);
}
