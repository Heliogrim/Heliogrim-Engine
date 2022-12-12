#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconAudio :
        public TextureAsset,
        public AssetAutoGuid<IconAudio> {
    public:
        IconAudio();
    };
}
