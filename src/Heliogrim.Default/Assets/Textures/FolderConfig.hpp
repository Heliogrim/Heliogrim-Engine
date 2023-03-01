#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderConfig :
        public TextureAsset,
        public AssetAutoGuid<FolderConfig> {
    public:
        FolderConfig();
    };
}
