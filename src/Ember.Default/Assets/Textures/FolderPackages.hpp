#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderPackages :
        public TextureAsset,
        public AssetAutoGuid<FolderPackages> {
    public:
        FolderPackages();
    };
}
