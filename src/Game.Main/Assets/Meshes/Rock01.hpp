#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class Rock01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Rock01> {
    public:
        Rock01();
    };
}
