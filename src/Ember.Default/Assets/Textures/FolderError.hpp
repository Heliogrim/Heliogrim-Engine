#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderError :
        public TextureAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError();
    };
}
