#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderLog :
        public TextureAsset,
        public AssetAutoGuid<FolderLog> {
    public:
        FolderLog();
    };
}
