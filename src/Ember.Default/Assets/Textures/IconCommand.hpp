#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconCommand :
        public TextureAsset,
        public AssetAutoGuid<IconCommand> {
    public:
        IconCommand();
    };
}
