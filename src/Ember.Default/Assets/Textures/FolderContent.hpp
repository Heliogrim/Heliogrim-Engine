#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderContent :
        public TextureAsset,
        public AssetAutoGuid<FolderContent> {
    public:
        FolderContent();
    };

    AUTO_REGISTER_ASSET(FolderContent)

}
