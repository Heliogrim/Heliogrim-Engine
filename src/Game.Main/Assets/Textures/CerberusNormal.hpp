#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class CerberusNormal :
        public TextureAsset,
        public AssetAutoGuid<CerberusNormal> {
    public:
        CerberusNormal();
    };
}
