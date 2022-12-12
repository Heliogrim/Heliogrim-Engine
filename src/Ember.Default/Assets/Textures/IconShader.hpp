#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconShader :
        public TextureAsset,
        public AssetAutoGuid<IconShader> {
    public:
        IconShader();
    };
}
