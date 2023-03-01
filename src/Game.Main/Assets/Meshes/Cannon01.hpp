#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Cannon01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cannon01> {
    public:
        Cannon01();
    };
}
