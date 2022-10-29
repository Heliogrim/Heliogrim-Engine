#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderImages :
        public TextureAsset,
        public AssetAutoGuid<FolderImages> {
    public:
        FolderImages();
    };

    AUTO_REGISTER_ASSET(FolderImages)

}
