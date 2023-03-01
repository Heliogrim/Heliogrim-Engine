#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Stick01AO :
        public TextureAsset,
        public AssetAutoGuid<Stick01AO> {
    public:
        Stick01AO();
    };
}
