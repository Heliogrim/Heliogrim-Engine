#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenPier01PolesNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal();
    };
}
