#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderMeta :
        public TextureAsset,
        public AssetAutoGuid<FolderMeta> {
    public:
        FolderMeta();
    };
}
