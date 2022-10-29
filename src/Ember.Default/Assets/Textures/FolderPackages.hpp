#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderPackages :
        public TextureAsset,
        public AssetAutoGuid<FolderPackages> {
    public:
        FolderPackages();
    };

    AUTO_REGISTER_ASSET(FolderPackages)

}
