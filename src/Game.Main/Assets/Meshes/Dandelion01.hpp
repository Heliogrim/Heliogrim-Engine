#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Dandelion01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Dandelion01> {
    public:
        Dandelion01();
    };
}
