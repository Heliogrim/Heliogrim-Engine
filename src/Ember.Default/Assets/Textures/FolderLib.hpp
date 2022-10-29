#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderLib :
        public TextureAsset,
        public AssetAutoGuid<FolderLib> {
    public:
        FolderLib();
    };

    AUTO_REGISTER_ASSET(FolderLib)

}
