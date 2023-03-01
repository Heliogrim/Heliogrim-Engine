#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderCore :
        public TextureAsset,
        public AssetAutoGuid<FolderCore> {
    public:
        FolderCore();
    };
}
