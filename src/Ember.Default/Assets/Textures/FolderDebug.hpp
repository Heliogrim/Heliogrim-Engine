#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderDebug :
        public TextureAsset,
        public AssetAutoGuid<FolderDebug> {
    public:
        FolderDebug();
    };

    AUTO_REGISTER_ASSET(FolderDebug)

}
