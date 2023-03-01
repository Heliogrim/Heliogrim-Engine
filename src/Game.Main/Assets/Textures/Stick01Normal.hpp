#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Stick01Normal :
        public TextureAsset,
        public AssetAutoGuid<Stick01Normal> {
    public:
        Stick01Normal();
    };
}
