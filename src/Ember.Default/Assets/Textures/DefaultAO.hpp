#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultAO :
        public TextureAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO();
    };
}
