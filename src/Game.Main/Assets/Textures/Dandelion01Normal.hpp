#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Dandelion01Normal :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Normal> {
    public:
        Dandelion01Normal();
    };
}
