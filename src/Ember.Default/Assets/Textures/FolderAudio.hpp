#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class FolderAudio :
        public TextureAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio();
    };
}
