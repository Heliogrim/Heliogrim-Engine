#pragma once
#include <Ember/StaticGeometryAsset.hpp>

namespace ember::game::assets::meshes {
    class Sphere :
        public StaticGeometryAsset,
        public AssetAutoGuid<Sphere> {
    public:
        Sphere();
    };
}
