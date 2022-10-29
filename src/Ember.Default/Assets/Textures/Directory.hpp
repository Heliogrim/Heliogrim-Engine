#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Directory :
        public TextureAsset,
        public AssetAutoGuid<Directory> {
    public:
        Directory();
    };

    AUTO_REGISTER_ASSET(Directory)

}
