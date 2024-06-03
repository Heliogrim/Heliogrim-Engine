#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderVideo :
        public TextureAsset,
        public AssetAutoGuid<FolderVideo> {
    public:
        FolderVideo();
    };
}
