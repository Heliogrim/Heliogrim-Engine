#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderDatabase :
        public TextureAsset,
        public AssetAutoGuid<FolderDatabase> {
    public:
        FolderDatabase();
    };
}
