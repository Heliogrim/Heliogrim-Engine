#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderResource :
        public TextureAsset,
        public AssetAutoGuid<FolderResource> {
    public:
        FolderResource();
    };

    AUTO_REGISTER_ASSET(FolderResource)

}
