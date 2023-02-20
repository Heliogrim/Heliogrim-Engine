#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class UIDummy :
        public TextureAsset,
        public AssetAutoGuid<UIDummy> {
    public:
        UIDummy();
    };
}
