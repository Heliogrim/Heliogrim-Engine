#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderImport :
        public TextureAsset,
        public AssetAutoGuid<FolderImport> {
    public:
        FolderImport();
    };
}
