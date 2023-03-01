#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderLib :
        public TextureAsset,
        public AssetAutoGuid<FolderLib> {
    public:
        FolderLib();
    };
}
