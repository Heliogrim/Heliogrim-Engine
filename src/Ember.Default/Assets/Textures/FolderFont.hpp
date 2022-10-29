#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderFont :
        public TextureAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont();
    };

    AUTO_REGISTER_ASSET(FolderFont)

}
