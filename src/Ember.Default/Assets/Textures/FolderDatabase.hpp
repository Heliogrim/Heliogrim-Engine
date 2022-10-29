#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderDatabase :
        public TextureAsset,
        public AssetAutoGuid<FolderDatabase> {
    public:
        FolderDatabase();
    };

    AUTO_REGISTER_ASSET(FolderDatabase)

}
