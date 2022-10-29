#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderShared :
        public TextureAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared();
    };

    AUTO_REGISTER_ASSET(FolderShared)

}
