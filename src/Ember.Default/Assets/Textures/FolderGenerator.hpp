#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderGenerator :
        public TextureAsset,
        public AssetAutoGuid<FolderGenerator> {
    public:
        FolderGenerator();
    };
}
