#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconFont :
        public TextureAsset,
        public AssetAutoGuid<IconFont> {
    public:
        IconFont();
    };
}
