#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconGit :
        public TextureAsset,
        public AssetAutoGuid<IconGit> {
    public:
        IconGit();
    };

    AUTO_REGISTER_ASSET(IconGit)

}
