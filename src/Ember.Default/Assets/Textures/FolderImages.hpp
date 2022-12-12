#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderImages :
        public TextureAsset,
        public AssetAutoGuid<FolderImages> {
    public:
        FolderImages();
    };
}
