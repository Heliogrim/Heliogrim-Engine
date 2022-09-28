#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class Cerberus :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cerberus> {
    public:
        Cerberus();
    };

    AUTO_REGISTER_ASSET(Cerberus);
}
