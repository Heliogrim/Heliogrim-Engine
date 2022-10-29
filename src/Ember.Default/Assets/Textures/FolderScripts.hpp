#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderScripts :
        public TextureAsset,
        public AssetAutoGuid<FolderScripts> {
    public:
        FolderScripts();
    };

    AUTO_REGISTER_ASSET(FolderScripts)

}
