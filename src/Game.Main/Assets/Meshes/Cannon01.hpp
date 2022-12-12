#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class Cannon01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cannon01> {
    public:
        Cannon01();
    };
}
