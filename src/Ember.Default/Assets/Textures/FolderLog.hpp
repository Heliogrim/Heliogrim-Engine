#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderLog :
        public TextureAsset,
        public AssetAutoGuid<FolderLog> {
    public:
        FolderLog();
    };

    AUTO_REGISTER_ASSET(FolderLog)

}
