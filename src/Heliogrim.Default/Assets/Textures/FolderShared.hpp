#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderShared :
        public TextureAsset,
        public AssetAutoGuid<FolderShared> {
    public:
        FolderShared();
    };
}
