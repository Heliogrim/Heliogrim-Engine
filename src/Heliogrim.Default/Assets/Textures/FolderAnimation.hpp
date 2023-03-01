#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderAnimation :
        public TextureAsset,
        public AssetAutoGuid<FolderAnimation> {
    public:
        FolderAnimation();
    };
}
