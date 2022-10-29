#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconTree :
        public TextureAsset,
        public AssetAutoGuid<IconTree> {
    public:
        IconTree();
    };

    AUTO_REGISTER_ASSET(IconTree)

}
