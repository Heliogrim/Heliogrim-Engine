#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderScripts :
        public TextureAsset,
        public AssetAutoGuid<FolderScripts> {
    public:
        FolderScripts();
    };
}
