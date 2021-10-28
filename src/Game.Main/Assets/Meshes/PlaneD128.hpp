#pragma once
#include <Ember/StaticMeshAsset.hpp>

namespace ember::game::assets::meshes {

    class PlaneD128 :
        public StaticMeshAsset,
        public AssetAutoGuid<PlaneD128> {
    public:
        PlaneD128();
    };

    AUTO_REGISTER_ASSET(PlaneD128);
}
