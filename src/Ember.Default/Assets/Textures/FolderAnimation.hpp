#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderAnimation :
        public TextureAsset,
        public AssetAutoGuid<FolderAnimation> {
    public:
        FolderAnimation();
    };
}
