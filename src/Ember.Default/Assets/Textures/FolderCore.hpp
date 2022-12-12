#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderCore :
        public TextureAsset,
        public AssetAutoGuid<FolderCore> {
    public:
        FolderCore();
    };
}
