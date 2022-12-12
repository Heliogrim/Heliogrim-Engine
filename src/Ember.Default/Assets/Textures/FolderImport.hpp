#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderImport :
        public TextureAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport();
    };
}
