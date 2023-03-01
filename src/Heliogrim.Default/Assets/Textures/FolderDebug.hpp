#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderDebug :
        public TextureAsset,
        public AssetAutoGuid<FolderDebug> {
    public:
        FolderDebug();
    };
}
