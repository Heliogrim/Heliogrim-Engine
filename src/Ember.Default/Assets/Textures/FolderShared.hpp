#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderShared :
        public TextureAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared();
    };
}
