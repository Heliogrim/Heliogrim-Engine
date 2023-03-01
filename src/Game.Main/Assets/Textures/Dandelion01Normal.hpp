#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Dandelion01Normal :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Normal> {
    public:
        Dandelion01Normal();
    };
}
