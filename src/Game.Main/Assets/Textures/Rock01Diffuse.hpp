#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Rock01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Rock01Diffuse> {
    public:
        Rock01Diffuse();
    };
}
