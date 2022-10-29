#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderEnvironment :
        public TextureAsset,
        public AssetAutoGuid<FolderEnvironment> {
    public:
        FolderEnvironment();
    };

    AUTO_REGISTER_ASSET(FolderEnvironment)

}
