#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconAudio :
        public TextureAsset,
        public AssetAutoGuid<IconAudio> {
    public:
        IconAudio();
    };
}
