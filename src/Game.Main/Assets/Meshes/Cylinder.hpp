#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Cylinder :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cylinder> {
    public:
        Cylinder();
    };
}
