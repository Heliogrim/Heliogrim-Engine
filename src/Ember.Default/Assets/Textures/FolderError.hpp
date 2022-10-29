#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderError :
        public TextureAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError();
    };

    AUTO_REGISTER_ASSET(FolderError)

}
