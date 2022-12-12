#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class WoodenPier01Poles :
        public StaticGeometryAsset,
        public AssetAutoGuid<WoodenPier01Poles> {
    public:
        WoodenPier01Poles();
    };
}
