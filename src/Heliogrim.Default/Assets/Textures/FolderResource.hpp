#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderResource :
        public TextureAsset,
        public AssetAutoGuid<FolderResource> {
    public:
        FolderResource();
    };
}
