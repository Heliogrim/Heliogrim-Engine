#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Directory :
        public TextureAsset,
        public AssetAutoGuid<Directory> {
    public:
        Directory();
    };
}
