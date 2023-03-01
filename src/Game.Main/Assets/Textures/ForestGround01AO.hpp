#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class ForestGround01AO :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO();
    };
}
