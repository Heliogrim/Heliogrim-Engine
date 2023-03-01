#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderPackages :
        public TextureAsset,
        public AssetAutoGuid<FolderPackages> {
    public:
        FolderPackages();
    };
}
