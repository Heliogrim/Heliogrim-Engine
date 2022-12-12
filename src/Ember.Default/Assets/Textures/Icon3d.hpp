#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Icon3d :
        public TextureAsset,
        public AssetAutoGuid<Icon3d> {
    public:
        Icon3d();
    };
}
