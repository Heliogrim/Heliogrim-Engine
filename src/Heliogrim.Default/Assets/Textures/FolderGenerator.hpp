#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderGenerator :
        public TextureAsset,
        public AssetAutoGuid<FolderGenerator> {
    public:
        FolderGenerator();
    };
}
