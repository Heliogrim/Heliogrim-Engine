#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class IconImage :
        public TextureAsset,
        public AssetAutoGuid<IconImage> {
    public:
        IconImage();
    };
}
