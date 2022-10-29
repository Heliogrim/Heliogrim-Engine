#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderTemplate :
        public TextureAsset,
        public AssetAutoGuid<FolderTemplate> {
    public:
        FolderTemplate();
    };

    AUTO_REGISTER_ASSET(FolderTemplate)

}
