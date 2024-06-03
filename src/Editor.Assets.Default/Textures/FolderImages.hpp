#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderImages :
        public TextureAsset,
        public AssetAutoGuid<FolderImages> {
    public:
        FolderImages();
    };
}
