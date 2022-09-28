#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class Stick01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Stick01> {
    public:
        Stick01();
    };

    AUTO_REGISTER_ASSET(Stick01);
}
