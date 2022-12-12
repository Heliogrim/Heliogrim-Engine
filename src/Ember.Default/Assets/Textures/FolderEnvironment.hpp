#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderEnvironment :
        public TextureAsset,
        public AssetAutoGuid<FolderEnvironment> {
    public:
        FolderEnvironment();
    };
}
