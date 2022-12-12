#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderLib :
        public TextureAsset,
        public AssetAutoGuid<FolderLib> {
    public:
        FolderLib();
    };
}
