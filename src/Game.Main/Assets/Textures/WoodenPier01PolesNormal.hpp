#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PolesNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal();
    };
}
