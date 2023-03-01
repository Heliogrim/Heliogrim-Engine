#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
    class Cerberus :
        public StaticGeometryAsset,
        public AssetAutoGuid<Cerberus> {
    public:
        Cerberus();
    };
}
