#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconDiff :
        public TextureAsset,
        public AssetAutoGuid<IconDiff> {
    public:
        IconDiff();
    };

    AUTO_REGISTER_ASSET(IconDiff)

}
