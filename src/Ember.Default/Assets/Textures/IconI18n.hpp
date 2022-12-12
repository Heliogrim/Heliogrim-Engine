#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconI18n :
        public TextureAsset,
        public AssetAutoGuid<IconI18n> {
    public:
        IconI18n();
    };
}
