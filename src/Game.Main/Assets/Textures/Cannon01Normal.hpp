#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Cannon01Normal :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Normal> {
    public:
        Cannon01Normal();
    };
}
