#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Sphere :
        public StaticGeometryAsset,
        public AssetAutoGuid<Sphere> {
    public:
        Sphere();
    };
}
