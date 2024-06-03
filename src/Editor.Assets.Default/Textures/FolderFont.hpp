#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderFont :
        public TextureAsset,
        public AssetAutoGuid<FolderFont> {
    public:
        FolderFont();
    };
}
