#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderGenerator :
        public TextureAsset,
        public AssetAutoGuid<FolderGenerator> {
    public:
        FolderGenerator();
    };

    AUTO_REGISTER_ASSET(FolderGenerator)

}
