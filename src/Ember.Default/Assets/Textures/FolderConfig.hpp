#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderConfig :
        public TextureAsset,
        public AssetAutoGuid<FolderConfig> {
    public:
        FolderConfig();
    };

    AUTO_REGISTER_ASSET(FolderConfig)

}
