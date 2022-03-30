#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class Cylinder :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cylinder> {
    public:
        Cylinder();
    };

    AUTO_REGISTER_ASSET(Cylinder);
}
