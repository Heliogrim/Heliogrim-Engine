#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderMeta :
        public TextureAsset,
        public AssetAutoGuid<FolderMeta> {
    public:
        FolderMeta();
    };
}
