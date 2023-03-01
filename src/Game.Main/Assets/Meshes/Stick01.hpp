#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Stick01 :
        public StaticGeometryAsset,
        public AssetAutoGuid<Stick01> {
    public:
        Stick01();
    };
}
