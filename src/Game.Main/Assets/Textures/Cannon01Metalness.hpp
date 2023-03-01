#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Cannon01Metalness :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Metalness> {
    public:
        Cannon01Metalness();
    };
}
