#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenPier01PolesDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesDiffuse> {
    public:
        WoodenPier01PolesDiffuse();
    };
}
