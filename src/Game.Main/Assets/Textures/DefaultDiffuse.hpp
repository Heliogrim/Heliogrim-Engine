#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultDiffuse :
        public TextureAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        DefaultDiffuse();
    };
}
