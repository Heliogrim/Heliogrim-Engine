#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Rock01Normal :
        public TextureAsset,
        public AssetAutoGuid<Rock01Normal> {
    public:
        Rock01Normal();
    };
}
