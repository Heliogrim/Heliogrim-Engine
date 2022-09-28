#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {

    class Dandelion01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Dandelion01> {
    public:
        Dandelion01();
    };

    AUTO_REGISTER_ASSET(Dandelion01);
}
