#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderVideo :
        public TextureAsset,
        public AssetAutoGuid<FolderVideo> {
    public:
        FolderVideo();
    };

    AUTO_REGISTER_ASSET(FolderVideo)

}
