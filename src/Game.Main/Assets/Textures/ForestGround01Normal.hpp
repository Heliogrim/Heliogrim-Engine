#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class ForestGround01Normal :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal();
    };
}
