#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconJson :
        public TextureAsset,
        public AssetAutoGuid<IconJson> {
    public:
        IconJson();
    };
}
