#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Rock01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Rock01> {
    public:
        Rock01();
    };
}
