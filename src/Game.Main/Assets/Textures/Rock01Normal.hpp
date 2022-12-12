#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Rock01Normal :
        public TextureAsset,
        public AssetAutoGuid<Rock01Normal> {
    public:
        Rock01Normal();
    };
}
