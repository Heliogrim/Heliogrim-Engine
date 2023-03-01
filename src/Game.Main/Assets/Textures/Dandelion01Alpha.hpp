#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Dandelion01Alpha :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Alpha> {
    public:
        Dandelion01Alpha();
    };
}
