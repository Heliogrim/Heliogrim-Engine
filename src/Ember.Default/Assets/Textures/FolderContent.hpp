#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderContent :
        public TextureAsset,
        public AssetAutoGuid<FolderContent> {
    public:
        FolderContent();
    };
}
