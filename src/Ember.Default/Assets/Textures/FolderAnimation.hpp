#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderAnimation :
        public TextureAsset,
        public AssetAutoGuid<FolderAnimation> {
    public:
        FolderAnimation();
    };

    AUTO_REGISTER_ASSET(FolderAnimation)

}
