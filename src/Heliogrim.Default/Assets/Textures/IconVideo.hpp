#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconVideo :
        public TextureAsset,
        public AssetAutoGuid<IconVideo> {
    public:
        IconVideo();
    };
}
