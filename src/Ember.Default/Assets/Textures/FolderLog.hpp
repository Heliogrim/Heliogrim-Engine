#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderLog :
        public TextureAsset,
        public AssetAutoGuid<FolderLog> {
    public:
        FolderLog();
    };
}
