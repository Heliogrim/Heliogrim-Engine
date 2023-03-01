#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Skybox01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Skybox01Diffuse> {
    public:
        Skybox01Diffuse();
    };
}
