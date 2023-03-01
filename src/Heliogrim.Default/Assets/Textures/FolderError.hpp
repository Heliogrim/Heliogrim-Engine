#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderError :
        public TextureAsset,
        public AssetAutoGuid<FolderError> {
    public:
        FolderError();
    };
}
