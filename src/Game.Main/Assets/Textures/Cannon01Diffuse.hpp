#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Cannon01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Diffuse> {
    public:
        Cannon01Diffuse();
    };
}
