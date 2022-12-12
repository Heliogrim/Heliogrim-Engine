#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderShader :
        public TextureAsset,
        public AssetAutoGuid<FolderShader> {
    public:
        FolderShader();
    };
}
