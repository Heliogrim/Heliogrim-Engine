#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderShader :
        public TextureAsset,
        public AssetAutoGuid<FolderShader> {
    public:
        FolderShader();
    };

    AUTO_REGISTER_ASSET(FolderShader)

}
