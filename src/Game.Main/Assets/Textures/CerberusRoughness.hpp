#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class CerberusRoughness :
        public TextureAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness();
    };
}
