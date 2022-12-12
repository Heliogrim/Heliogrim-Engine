#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderScripts :
        public TextureAsset,
        public AssetAutoGuid<FolderScripts> {
    public:
        FolderScripts();
    };
}
