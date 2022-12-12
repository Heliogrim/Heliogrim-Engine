#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class CubeD1 :
        public StaticGeometryAsset,
        public AssetAutoGuid<CubeD1> {
    public:
        CubeD1();
    };
}
