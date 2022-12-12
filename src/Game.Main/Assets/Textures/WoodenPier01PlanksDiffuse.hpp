#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PlanksDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksDiffuse> {
    public:
        WoodenPier01PlanksDiffuse();
    };
}
