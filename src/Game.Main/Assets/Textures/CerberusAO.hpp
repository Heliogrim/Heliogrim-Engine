#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class CerberusAO :
        public TextureAsset,
        public AssetAutoGuid<CerberusAO> {
    public:
        CerberusAO();
    };
}
