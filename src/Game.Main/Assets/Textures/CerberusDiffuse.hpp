#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class CerberusDiffuse :
        public TextureAsset,
        public AssetAutoGuid<CerberusDiffuse> {
    public:
        CerberusDiffuse();
    };
}
