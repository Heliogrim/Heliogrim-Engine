#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class PlaneD128 :
        public StaticGeometryAsset,
        public AssetAutoGuid<PlaneD128> {
    public:
        PlaneD128();
    };

    AUTO_REGISTER_ASSET(PlaneD128);
}
