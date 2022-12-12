#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconArchitecture :
        public TextureAsset,
        public AssetAutoGuid<IconArchitecture> {
    public:
        IconArchitecture();
    };
}
