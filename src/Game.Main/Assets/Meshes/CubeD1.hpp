#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class CubeD1 :
        public StaticGeometryAsset,
        public AssetAutoGuid<CubeD1> {
    public:
        CubeD1();
    };
}
