#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderFont :
        public TextureAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont();
    };
}
