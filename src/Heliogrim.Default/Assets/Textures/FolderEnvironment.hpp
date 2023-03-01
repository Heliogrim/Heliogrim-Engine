#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderEnvironment :
        public TextureAsset,
        public AssetAutoGuid<FolderEnvironment> {
    public:
        FolderEnvironment();
    };
}
