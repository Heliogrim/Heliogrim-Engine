#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class FolderAudio :
        public TextureAsset,
        public AssetAutoGuid<FolderAudio> {
    public:
        FolderAudio();
    };

    AUTO_REGISTER_ASSET(FolderAudio)

}
