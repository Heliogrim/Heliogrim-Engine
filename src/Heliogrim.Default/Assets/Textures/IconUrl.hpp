#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconUrl :
        public TextureAsset,
        public AssetAutoGuid<IconUrl> {
    public:
        IconUrl();
    };
}
