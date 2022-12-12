#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderVideo :
        public TextureAsset,
        public AssetAutoGuid<FolderVideo> {
    public:
        FolderVideo();
    };
}
