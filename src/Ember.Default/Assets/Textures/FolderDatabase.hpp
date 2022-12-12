#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderDatabase :
        public TextureAsset,
        public AssetAutoGuid<FolderDatabase> {
    public:
        FolderDatabase();
    };
}
