#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderTemplate :
        public TextureAsset,
        public AssetAutoGuid<FolderTemplate> {
    public:
        FolderTemplate();
    };
}
