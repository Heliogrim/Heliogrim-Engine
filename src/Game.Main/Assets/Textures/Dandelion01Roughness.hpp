#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Dandelion01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Roughness> {
    public:
        Dandelion01Roughness();
    };
}
