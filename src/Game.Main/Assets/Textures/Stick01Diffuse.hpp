#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Stick01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Stick01Diffuse> {
    public:
        Stick01Diffuse();
    };
}
