#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DryGroundRocks01Normal :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Normal> {
    public:
        DryGroundRocks01Normal();
    };
}
