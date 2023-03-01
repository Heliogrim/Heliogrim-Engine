#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Dandelion01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Diffuse> {
    public:
        Dandelion01Diffuse();
    };
}
