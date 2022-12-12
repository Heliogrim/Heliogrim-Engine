#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Brand :
        public TextureAsset,
        public AssetAutoGuid<Brand> {
    public:
        Brand();
    };
}
