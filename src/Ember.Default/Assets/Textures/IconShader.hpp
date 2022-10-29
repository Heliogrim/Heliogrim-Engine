#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconShader :
        public TextureAsset,
        public AssetAutoGuid<IconShader> {
    public:
        IconShader();
    };

    AUTO_REGISTER_ASSET(IconShader)

}
