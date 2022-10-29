#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderImport :
        public TextureAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport();
    };

    AUTO_REGISTER_ASSET(FolderImport)

}
