#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderCore :
        public TextureAsset,
        public AssetAutoGuid<FolderCore> {
    public:
        FolderCore();
    };

    AUTO_REGISTER_ASSET(FolderCore)

}
