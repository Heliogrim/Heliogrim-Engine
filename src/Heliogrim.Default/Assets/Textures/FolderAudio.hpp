#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class FolderAudio :
        public TextureAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio();
    };
}
