#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconFavicon :
        public TextureAsset,
        public AssetAutoGuid<IconFavicon> {
    public:
        IconFavicon();
    };
}
